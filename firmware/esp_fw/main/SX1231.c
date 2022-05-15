#include "SX1231.h"
#include "SX1231_Registers.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_log.h>

static const char TAG[] = "SX1231";

#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t sx1231_isr_queue = NULL;

static void IRAM_ATTR sx1231_isr(void* arg)
{
  uint32_t gpio_num = (uint32_t) arg;
  xQueueSendFromISR(sx1231_isr_queue, &gpio_num, NULL);
}

static void sx1231_isr_task(void* arg)
{
  SX1231_t* sx1231 = (SX1231_t*)arg;
  uint32_t io_num;
  for(;;) {
    if(xQueueReceive(sx1231_isr_queue, &io_num, portMAX_DELAY)) {
      if(io_num == sx1231->cfg.gpio_int)
        sx1231->_dataAvailable = true;
    }
  }
}

esp_err_t sx1231_install_interrupts(SX1231_t* ctx) 
{
  //zero-initialize the config structure.
  gpio_config_t io_conf = {};

  //interrupt of rising edge
  io_conf.intr_type = GPIO_INTR_POSEDGE;
  //bit mask of the pins, use GPIO_INPUT_PIN_SEL here
  io_conf.pin_bit_mask = (1ULL<<ctx->cfg.gpio_int);
  //set as input mode
  io_conf.mode = GPIO_MODE_INPUT;
  //enable pull-down mode
  io_conf.pull_down_en = 1;
  gpio_config(&io_conf);

  //create a queue to handle gpio event from isr
  sx1231_isr_queue = xQueueCreate(10, sizeof(uint32_t));
  //start gpio task
  xTaskCreate(sx1231_isr_task, "sx1231_isr_task", 2048, ctx, 10, NULL);

  //install gpio isr service
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  //hook isr handler for specific gpio pin
  gpio_isr_handler_add(ctx->cfg.gpio_int, sx1231_isr, (void*) ctx->cfg.gpio_int);
  return ESP_OK;
}

esp_err_t sx1231_reset(SX1231_t* ctx)
{
  gpio_set_direction(ctx->cfg.gpio_reset, GPIO_MODE_OUTPUT);
  gpio_set_level(ctx->cfg.gpio_reset, 1);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  gpio_set_level(ctx->cfg.gpio_reset, 0);
  vTaskDelay(10 / portTICK_PERIOD_MS);
  return ESP_OK;
}

esp_err_t sx1231_initialize(SX1231_config_t *cfg, SX1231_t** out_ctx) 
{
  esp_err_t err = ESP_OK;
  spi_device_handle_t spi;

  spi_device_interface_config_t devcfg={
    .clock_speed_hz = SPI_MASTER_FREQ_10M,
    .mode = 0,  //SPI mode 0
    .spics_io_num = cfg->gpio_cs,
    .queue_size = 8,
    .command_bits = 8,
    .input_delay_ns=100,
    .flags = SPI_DEVICE_NO_DUMMY
  };

  //Attach the radio to the SPI bus
  err = spi_bus_add_device(cfg->host, &devcfg, &spi);

  if (err != ESP_OK)  {
    ESP_LOGE(TAG, "Could not create SPI device");
    return err;
  } // TODO: should cleanup here

  SX1231_t* ctx = (SX1231_t*)malloc(sizeof(SX1231_t));
  if (!ctx) return ESP_ERR_NO_MEM;

  *ctx = (SX1231_t) {
    .cfg = *cfg,
    ._mode = RF69_MODE_STANDBY,
    ._powerLevel = 31,
    ._dataAvailable = false,
    ._address = cfg->nodeID,
    .spi = spi 
  };
  sx1231_reset(ctx);

  // this is a lot of data. maybe put it in flash?
  const uint8_t CONFIG[][2] =
  {
    /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
    /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
    /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_55555}, // default: 4.8 KBPS
    /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_55555},
    /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_50000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
    /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_50000},

    /* 0x07 */ { REG_FRFMSB, (uint8_t) (cfg->freqBand==RF69_315MHZ ? RF_FRFMSB_315 : (cfg->freqBand==RF69_433MHZ ? RF_FRFMSB_433 : (cfg->freqBand==RF69_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
    /* 0x08 */ { REG_FRFMID, (uint8_t) (cfg->freqBand==RF69_315MHZ ? RF_FRFMID_315 : (cfg->freqBand==RF69_433MHZ ? RF_FRFMID_433 : (cfg->freqBand==RF69_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
    /* 0x09 */ { REG_FRFLSB, (uint8_t) (cfg->freqBand==RF69_315MHZ ? RF_FRFLSB_315 : (cfg->freqBand==RF69_433MHZ ? RF_FRFLSB_433 : (cfg->freqBand==RF69_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },

    // looks like PA1 and PA2 are not implemented on RFM69W/CW, hence the max output power is 13dBm
    // +17dBm and +20dBm are possible on RFM69HW
    // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
    // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
    // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
    ///* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
    ///* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 }, // over current protection (default is 95mA)

    // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
    /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
    //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
    /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
    /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
    /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
    /* 0x29 */ { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
    ///* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
    /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
    /* 0x2F */ { REG_SYNCVALUE1, 0x2D },      // attempt to make this compatible with sync1 byte of RFM12B lib
    /* 0x30 */ { REG_SYNCVALUE2, cfg->networkID }, // NETWORK ID
    //* 0x31 */ { REG_SYNCVALUE3, 0xAA },
    //* 0x31 */ { REG_SYNCVALUE4, 0xBB },
    /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
    /* 0x38 */ { REG_PAYLOADLENGTH, 66 }, // in variable length mode: the max frame size, not used in TX
    ///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
    /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
    /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_OFF | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    //for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
    {255, 0}
  };

  // esp timer is actually a uint64_t
  uint64_t start = (esp_timer_get_time() / 1000);
  uint8_t timeout = 50;

  do sx1231_writeReg(ctx, REG_SYNCVALUE1, 0xAA); while (sx1231_readReg(ctx, REG_SYNCVALUE1) != 0xAA && (esp_timer_get_time() / 1000)-start < timeout);
  start = (esp_timer_get_time() / 1000);
  do sx1231_writeReg(ctx, REG_SYNCVALUE1, 0x55); while (sx1231_readReg(ctx, REG_SYNCVALUE1) != 0x55 && (esp_timer_get_time() / 1000)-start < timeout);
  ESP_LOGI(TAG, "Radio sync");

  for (uint8_t i = 0; CONFIG[i][0] != 255; i++)
    sx1231_writeReg(ctx, CONFIG[i][0], CONFIG[i][1]);

  // Encryption is persistent between resets and can trip you up during debugging.
  // Disable it during initialization so we always start from a known state.
  sx1231_encrypt(ctx, 0);

  sx1231_setHighPower(ctx, cfg->isRFM69HW_HCW); // called regardless if it's a RFM69W or RFM69HW (at this point _isRFM69HW may not be explicitly set by constructor and setHighPower() may not have been called yet (ie called after initialize() call)
  sx1231_setMode(ctx, RF69_MODE_STANDBY);
  start = (esp_timer_get_time() / 1000);

  while (((sx1231_readReg(ctx, REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00) && (esp_timer_get_time() / 1000)-start < timeout); // wait for ModeReady

  if ((esp_timer_get_time() / 1000)-start >= timeout)
    return -1;

  err = sx1231_install_interrupts(ctx);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "Could not attach interrupts");
    return err;
  }

  *out_ctx = ctx;
  return ESP_OK;
}

void sx1231_setAddress(SX1231_t *sx1231, uint16_t addr) {
  sx1231->_address = addr;
  sx1231_writeReg(sx1231, REG_NODEADRS, sx1231->_address); //unused in packet mode
}

void sx1231_setNetwork(SX1231_t *sx1231, uint8_t networkID) {
  sx1231_writeReg(sx1231, REG_SYNCVALUE2, networkID);
}

bool sx1231_canSend(SX1231_t *sx1231) {
  if (sx1231->_mode == RF69_MODE_RX && sx1231->PAYLOADLEN == 0 && sx1231_readRSSI(sx1231, false) < CSMA_LIMIT) // if signal stronger than -100dBm is detected assume channel activity
  {
    sx1231_setMode(sx1231, RF69_MODE_STANDBY);
    return true;
  }
  return false;
}

void sx1231_send(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK) {
  sx1231_writeReg(sx1231, REG_PACKETCONFIG2, (sx1231_readReg(sx1231, REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  uint32_t now = (esp_timer_get_time() / 1000);
  while (!sx1231_canSend(sx1231) && (esp_timer_get_time() / 1000) - now < RF69_CSMA_LIMIT_MS){
      sx1231_receiveDone(sx1231);
  }
  sx1231_sendFrame(sx1231, toAddress, buffer, bufferSize, requestACK, false);
}

// to increase the chance of getting a packet across, call this function instead of send
// and it handles all the ACK requesting/retrying for you :)
// The only twist is that you have to manually listen to ACK requests on the other side and send back the ACKs
// The reason for the semi-automaton is that the lib is interrupt driven and
// requires user action to read the received data and decide what to do with it
// replies usually take only 5..8ms at 50kbps@915MHz
bool sx1231_sendWithRetry(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries, uint8_t retryWaitTime) {
  uint32_t sentTime;
  for (uint8_t i = 0; i <= retries; i++)
  {
    sx1231_send(sx1231, toAddress, buffer, bufferSize, true);
    sentTime = (esp_timer_get_time() / 1000);
    while ((esp_timer_get_time() / 1000) - sentTime < retryWaitTime)
    {
      if (sx1231_ACKReceived(sx1231, toAddress)) return true;
      vTaskDelay(1);
    }
  }
  return false;
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool sx1231_receiveDone(SX1231_t *sx1231) {
  if (sx1231->_dataAvailable) {
  	sx1231->_dataAvailable = false;
  	sx1231_interruptHandler(sx1231);
  }

  if (sx1231->_mode == RF69_MODE_RX && sx1231->PAYLOADLEN > 0)
  {
    sx1231_setMode(sx1231, RF69_MODE_STANDBY); // enables interrupts
    return true;
  }

  else if (sx1231->_mode == RF69_MODE_RX) // already in RX no payload yet
    return false;

  sx1231_receiveBegin(sx1231);
  return false;
}

// should be polled immediately after sending a packet with ACK request
bool sx1231_ACKReceived(SX1231_t *sx1231, uint16_t fromNodeID) {
  if (sx1231_receiveDone(sx1231))
    return (sx1231->SENDERID == fromNodeID || fromNodeID == RF69_BROADCAST_ADDR) && sx1231->ACK_RECEIVED;
  return false;
}

// check whether an ACK was requested in the last received packet (non-broadcasted packet)
bool sx1231_ACKRequested(SX1231_t *sx1231) {
  return sx1231->ACK_REQUESTED && (sx1231->TARGETID == sx1231->_address);
}

// should be called immediately after reception in case sender wants ACK
void sx1231_sendACK(SX1231_t *sx1231, const void* buffer, uint8_t bufferSize) {
  sx1231->ACK_REQUESTED = 0;   // TWS added to make sure we don't end up in a timing race and infinite loop sending Acks
  uint16_t sender = sx1231->SENDERID;
  int16_t _RSSI = sx1231->RSSI; // save payload received RSSI value
  sx1231_writeReg(sx1231, REG_PACKETCONFIG2, (sx1231_readReg(sx1231, REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  uint32_t now = (esp_timer_get_time() / 1000);
  while (!sx1231_canSend(sx1231) && (esp_timer_get_time() / 1000) - now < RF69_CSMA_LIMIT_MS){
      sx1231_receiveDone(sx1231);
  }
  sx1231->SENDERID = sender;    // TWS: Restore SenderID after it gets wiped out by receiveDone()
  sx1231_sendFrame(sx1231, sender, buffer, bufferSize, false, true);
  sx1231->RSSI = _RSSI; // restore payload RSSI
}

uint32_t sx1231_getFrequency(SX1231_t *sx1231) {
  return RF69_FSTEP * (((uint32_t) sx1231_readReg(sx1231, REG_FRFMSB) << 16) + ((uint16_t) sx1231_readReg(sx1231, REG_FRFMID) << 8) + sx1231_readReg(sx1231, REG_FRFLSB));
}

void sx1231_setFrequency(SX1231_t *sx1231, uint32_t freqHz) {
  uint8_t oldMode = sx1231->_mode;
  if (oldMode == RF69_MODE_TX) {
    sx1231_setMode(sx1231, RF69_MODE_RX);
  }
  freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
  sx1231_writeReg(sx1231, REG_FRFMSB, freqHz >> 16);
  sx1231_writeReg(sx1231, REG_FRFMID, freqHz >> 8);
  sx1231_writeReg(sx1231, REG_FRFLSB, freqHz);
  if (oldMode == RF69_MODE_RX) {
    sx1231_setMode(sx1231, RF69_MODE_SYNTH);
  }
  sx1231_setMode(sx1231, oldMode);
}

// To enable encryption: radio.encrypt("ABCDEFGHIJKLMNOP");
// To disable encryption: radio.encrypt(null) or radio.encrypt(0)
// KEY HAS TO BE 16 bytes !!!
void sx1231_encrypt(SX1231_t *sx1231, const char* key) {
  sx1231_setMode(sx1231, RF69_MODE_STANDBY);
  uint8_t validKey = key != 0 && strlen(key)!=0;

  if (validKey)
  {
    sx1231_select(sx1231);
    // _spi->transfer(REG_AESKEY1 | 0x80);
    spi_transaction_t t = {
      .cmd = REG_AESKEY1 | 0x80,
      .length = 16,
      .flags = 0,
      .tx_buffer = key,
      .user = sx1231,
    };
    esp_err_t err = spi_device_polling_transmit(sx1231->spi, &t);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not transmit encrypt key");
      return;
    }
    sx1231_unselect(sx1231);
  }
  sx1231_writeReg(sx1231, REG_PACKETCONFIG2, (sx1231_readReg(sx1231, REG_PACKETCONFIG2) & 0xFE) | (validKey ? 1 : 0));
}

int16_t sx1231_readRSSI(SX1231_t *sx1231, bool forceTrigger) {
  int16_t rssi = 0;
  if (forceTrigger)
  {
    // RSSI trigger not needed if DAGC is in continuous mode
    sx1231_writeReg(sx1231, REG_RSSICONFIG, RF_RSSI_START);
    while ((sx1231_readReg(sx1231, REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // wait for RSSI_Ready
  }
  rssi = -sx1231_readReg(sx1231, REG_RSSIVALUE);
  rssi >>= 1;
  return rssi;
} 

// for RFM69 HW/HCW only: you must call setHighPower(true) after initialize() or else transmission won't work
void sx1231_setHighPower(SX1231_t *sx1231, bool isRFM69HW_HCW) {
    sx1231->cfg.isRFM69HW_HCW = isRFM69HW_HCW;
    sx1231_writeReg(sx1231, REG_OCP, sx1231->cfg.isRFM69HW_HCW ? RF_OCP_OFF : RF_OCP_ON); //disable OverCurrentProtection for HW/HCW
    sx1231_setPowerLevel(sx1231, sx1231->_powerLevel);
}

// Control transmitter output power (this is NOT a dBm value!)
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//   - for RFM69 W/CW the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//   - for RFM69 HW/HCW the range is from 0-22 [-2dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
//   - the HW/HCW 0-24 range is split into 3 REG_PALEVEL parts:
//     -  0-15 = REG_PALEVEL 16-31, ie [-2 to 13dBm] & PA1 only
//     - 16-19 = REG_PALEVEL 26-29, ie [12 to 15dBm] & PA1+PA2
//     - 20-23 = REG_PALEVEL 28-31, ie [17 to 20dBm] & PA1+PA2+HiPower (HiPower is only enabled before going in TX mode, ie by setMode(RF69_MODE_TX)
// The HW/HCW range overlaps are to smooth out transitions between the 3 PA domains, based on actual current/RSSI measurements
// Any changes to this function also demand changes in dependent function setPowerDBm()
void sx1231_setPowerLevel(SX1231_t *sx1231, uint8_t powerLevel) {
  uint8_t PA_SETTING;
  if (sx1231->cfg.isRFM69HW_HCW) {
    if (powerLevel>23) powerLevel = 23;
    sx1231->_powerLevel =  powerLevel;

    //now set Pout value & active PAs based on _powerLevel range as outlined in summary above
    if (sx1231->_powerLevel < 16) {
      powerLevel += 16;
      PA_SETTING = RF_PALEVEL_PA1_ON; // enable PA1 only
    } else {
      if (sx1231->_powerLevel < 20)
        powerLevel += 10;
      else 
        powerLevel += 8;
      PA_SETTING = RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON; // enable PA1+PA2
    }
    sx1231_setHighPowerRegs(sx1231, true); //always call this in case we're crossing power boundaries in TX mode
  } else { //this is a W/CW, register value is the same as _powerLevel
    if (powerLevel>31) powerLevel = 31;
    sx1231->_powerLevel =  powerLevel;
    PA_SETTING = RF_PALEVEL_PA0_ON; // enable PA0 only
  }

  //write value to REG_PALEVEL
  sx1231_writeReg(sx1231, REG_PALEVEL, PA_SETTING | powerLevel);
}

//Set TX Output power in dBm:
// [-18..+13]dBm in RFM69 W/CW
// [ -2..+20]dBm in RFM69 HW/HCW
int8_t sx1231_setPowerDBm(SX1231_t *sx1231, int8_t dBm) {
  if (sx1231->cfg.isRFM69HW_HCW) {
    //fix any out of bounds
    if (dBm<-2) dBm=-2;
    else if (dBm>20) dBm=20;

    //map dBm to _powerLevel according to implementation in sx1231_setPowerLevel(sx1231, )
    if (dBm<12) sx1231_setPowerLevel(sx1231, 2+dBm);
    else if (dBm<16) sx1231_setPowerLevel(sx1231, 4+dBm);
    else sx1231_setPowerLevel(sx1231, 3+dBm);
  } else { //W/CW
    if (dBm<-18) dBm=-18;
    else if (dBm>13) dBm=13;
    sx1231_setPowerLevel(sx1231, 18+dBm);
  }
  return dBm;
}

uint8_t sx1231_getPowerLevel(SX1231_t *sx1231) {
  return sx1231->_powerLevel;
} 

void sx1231_sleep(SX1231_t *sx1231) {
  sx1231_setMode(sx1231, RF69_MODE_SLEEP);
}

uint8_t sx1231_readTemperature(SX1231_t *sx1231, uint8_t calFactor) {
  sx1231_setMode(sx1231, RF69_MODE_STANDBY);
  sx1231_writeReg(sx1231, REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((sx1231_readReg(sx1231, REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~sx1231_readReg(sx1231, REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
} 

// calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]
void sx1231_rcCalibration(SX1231_t *sx1231) {
  sx1231_writeReg(sx1231, REG_OSC1, RF_OSC1_RCCAL_START);
  while ((sx1231_readReg(sx1231, REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}

void sx1231_set300KBPS(SX1231_t *sx1231) {
  sx1231_writeReg(sx1231, 0x03, 0x00);  //REG_BITRATEMSB: 300kbps (0x006B, see DS p20)
  sx1231_writeReg(sx1231, 0x04, 0x6B);  //REG_BITRATELSB: 300kbps (0x006B, see DS p20)
  sx1231_writeReg(sx1231, 0x19, 0x40);  //REG_RXBW: 500kHz
  sx1231_writeReg(sx1231, 0x1A, 0x80);  //REG_AFCBW: 500kHz
  sx1231_writeReg(sx1231, 0x05, 0x13);  //REG_FDEVMSB: 300khz (0x1333)
  sx1231_writeReg(sx1231, 0x06, 0x33);  //REG_FDEVLSB: 300khz (0x1333)
  sx1231_writeReg(sx1231, 0x29, 240);   //set REG_RSSITHRESH to -120dBm
  sx1231_writeReg(sx1231, 0x37, 0b10010000); //DC=WHITENING, CRCAUTOOFF=0
  //                ^^->DC: 00=none, 01=manchester, 10=whitening
}

//=============================================================================
// setLNA() - disable the AGC and set a manual gain to attenuate input signal
// Makes receiver hear a "weaker" signal.
// Use this function to simulate a receiver "distance" from a transmitter
// newReg should be: (see table 26 RegLna 0x18 values)
//  000 - gain set by the internal AGC loop (when bits 
//  001 - G1 = highest gain
//  010 - G2 = highest gain 6 dB
//  011 - G3 = highest gain 12 dB
//  100 - G4 = highest gain 24 dB
//  101 - G5 = highest gain 36 dB
//  110 - G6 = highest gain 48 dB
//  111 - reserved
//=============================================================================
uint8_t sx1231_setLNA(SX1231_t *sx1231, uint8_t newReg) {
  uint8_t oldReg;
  oldReg = sx1231_readReg(sx1231, REG_LNA);
  sx1231_writeReg(sx1231, REG_LNA, ((newReg & 7) | (oldReg & ~7))); // just control the LNA Gain bits for now
  return oldReg;  // return the original value in case we need to restore it
}

uint8_t sx1231_readReg(SX1231_t *sx1231, uint8_t addr) 
{
  sx1231_select(sx1231);
  spi_transaction_t t = {
    .cmd = addr & 0x7F,
    .length = 8,
    .flags = SPI_TRANS_USE_RXDATA,
    .user = sx1231,
  };

  esp_err_t err = spi_device_polling_transmit(sx1231->spi, &t);
  if (err!= ESP_OK) {
    ESP_LOGE(TAG, "Could not read register %02X", addr);
    return 0; // this is wrong lmao
  }

  uint8_t regval = t.rx_data[0];
  sx1231_unselect(sx1231);
  return regval;
}

void sx1231_writeReg(SX1231_t *sx1231, uint8_t addr, uint8_t value) 
{
  sx1231_select(sx1231);
  spi_transaction_t t = {
    .cmd = addr | 0x80,
    .length = 8,
    .flags = SPI_TRANS_USE_TXDATA,
    .tx_data = {value},
    .user = sx1231,
  };
  esp_err_t err = spi_device_polling_transmit(sx1231->spi, &t);
  if (err!= ESP_OK) {
    ESP_LOGE(TAG, "Failed to write register %02X", addr);
    return;
  }

  sx1231_unselect(sx1231);
}

void sx1231_setMode(SX1231_t *sx1231, uint8_t newMode) {
  if (newMode == sx1231->_mode)
    return;

  switch (newMode) {
    case RF69_MODE_TX:
      sx1231_writeReg(sx1231, REG_OPMODE, (sx1231_readReg(sx1231, REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
      if (sx1231->cfg.isRFM69HW_HCW) sx1231_setHighPowerRegs(sx1231, true);
      break;
    case RF69_MODE_RX:
      sx1231_writeReg(sx1231, REG_OPMODE, (sx1231_readReg(sx1231, REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
      if (sx1231->cfg.isRFM69HW_HCW) sx1231_setHighPowerRegs(sx1231, false);
      break;
    case RF69_MODE_SYNTH:
      sx1231_writeReg(sx1231, REG_OPMODE, (sx1231_readReg(sx1231, REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
      break;
    case RF69_MODE_STANDBY:
      sx1231_writeReg(sx1231, REG_OPMODE, (sx1231_readReg(sx1231, REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
      break;
    case RF69_MODE_SLEEP:
      sx1231_writeReg(sx1231, REG_OPMODE, (sx1231_readReg(sx1231, REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
      break;
    default:
      return;
  }

  // we are using packet mode, so this check is not really needed
  // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
  while (sx1231->_mode == RF69_MODE_SLEEP && (sx1231_readReg(sx1231, REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady

  sx1231->_mode = newMode;
}

void sx1231_select(SX1231_t *sx1231) {
  esp_err_t err;
  err = spi_device_acquire_bus(sx1231->spi, portMAX_DELAY);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Could not aquire SPI bus");
    return;
  }
}

void sx1231_unselect(SX1231_t *sx1231) {
  spi_device_release_bus(sx1231->spi);
}

// Private API

void sx1231_interruptHandler(SX1231_t *sx1231) {
  esp_err_t err;
  if (sx1231->_mode == RF69_MODE_RX && (sx1231_readReg(sx1231, REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY))
  {
    sx1231_setMode(sx1231, RF69_MODE_STANDBY);

    sx1231_select(sx1231);

    spi_transaction_t t1 = {
        .cmd = REG_FIFO & 0x7F,
        .length = 32,
        .flags = SPI_TRANS_USE_RXDATA,
        .user = sx1231,
    };
    err = spi_device_polling_transmit(sx1231->spi, &t1);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not read SPI bus");
      return;
    }
    sx1231->PAYLOADLEN = t1.rx_data[0];
    sx1231->PAYLOADLEN = sx1231->PAYLOADLEN > 66 ? 66 : sx1231->PAYLOADLEN; // precaution
    sx1231->TARGETID = t1.rx_data[1];
    sx1231->SENDERID = t1.rx_data[2];
    uint8_t CTLbyte = t1.rx_data[3];

    sx1231->TARGETID |= (((uint16_t) CTLbyte) & 0x0C) << 6; //10 bit address (most significant 2 bits stored in bits(2,3) of CTL byte
    sx1231->SENDERID |= (((uint16_t) CTLbyte) & 0x03) << 8; //10 bit address (most sifnigicant 2 bits stored in bits(0,1) of CTL byte

    if(!(sx1231->TARGETID == sx1231->_address || sx1231->TARGETID == RF69_BROADCAST_ADDR) // match this node's address, or broadcast address or anything in spy mode
       || sx1231->PAYLOADLEN < 3) // address situation could receive packets that are malformed and don't fit this libraries extra fields
    {
      sx1231->PAYLOADLEN = 0;
      sx1231_unselect(sx1231);
      sx1231_receiveBegin(sx1231);
      return;
    }

    sx1231->DATALEN = sx1231->PAYLOADLEN - 3;
    sx1231->ACK_RECEIVED = CTLbyte & RFM69_CTL_SENDACK; // extract ACK-received flag
    sx1231->ACK_REQUESTED = CTLbyte & RFM69_CTL_REQACK; // extract ACK-requested flag
    // sx1231_unselect(sx1231);

    ESP_LOGI(TAG, "CTLbyte=%d ACK=%d ACKR=%d", CTLbyte, sx1231->ACK_RECEIVED, sx1231->ACK_REQUESTED);
    // while(1) {
    //   vTaskDelay(10);
    // }
    // sx1231_select(sx1231);

    spi_transaction_t t2 = {
      .cmd = REG_FIFO & 0x7F,
      .length = sx1231->DATALEN * 8,
      .user = sx1231,
      .rx_buffer = sx1231->DATA
    };

    err = spi_device_polling_transmit(sx1231->spi, &t2);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Could not read payload");
      return;
    }

    sx1231->DATA[sx1231->DATALEN] = 0; // add null at end of payload 
    sx1231_unselect(sx1231);
    sx1231_setMode(sx1231, RF69_MODE_RX);
  }
  sx1231->RSSI = sx1231_readRSSI(sx1231, false);
}

void sx1231_sendFrame(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK, bool sendACK)
{
  sx1231_setMode(sx1231, RF69_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
  while ((sx1231_readReg(sx1231, REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
  //writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
  if (bufferSize > RF69_MAX_DATA_LEN) bufferSize = RF69_MAX_DATA_LEN;

  // control byte
  uint8_t CTLbyte = 0x00;
  if (sendACK)
    CTLbyte = RFM69_CTL_SENDACK;
  else if (requestACK)
    CTLbyte = RFM69_CTL_REQACK;

  if (toAddress > 0xFF) CTLbyte |= (toAddress & 0x300) >> 6; //assign last 2 bits of address if > 255
  if (sx1231->_address > 0xFF) CTLbyte |= (sx1231->_address & 0x300) >> 8;   //assign last 2 bits of address if > 255

  // write to FIFO
  sx1231_select(sx1231);
  char buffer2[4 + bufferSize];
  memset(buffer2, 0, 4 + bufferSize);
  buffer2[0] = bufferSize + 3;
  buffer2[1] = (uint8_t)toAddress;
  buffer2[2] = (uint8_t)sx1231->_address;
  buffer2[3] = CTLbyte;
  for(uint8_t i = 0; i < bufferSize; i++)
    buffer2[4 + i] = ((uint8_t*)buffer)[i];

  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t)); 
  t.cmd = REG_FIFO | 0x80;
  t.length=(bufferSize + 4)*8;
  t.tx_buffer=buffer2;
  t.user=sx1231;                
  ret=spi_device_polling_transmit(sx1231->spi, &t);  //Transmit!

  // _spi->transfer(REG_FIFO | 0x80);
  // _spi->transfer(bufferSize + 3);
  // _spi->transfer((uint8_t)toAddress);
  // _spi->transfer((uint8_t)sx1231->_address);
  // _spi->transfer(CTLbyte);

  // for (uint8_t i = 0; i < bufferSize; i++)
  //   _spi->transfer(((uint8_t*) buffer)[i]);
  // ESP_LOGI(TAG, "header = {%02X, %02X, %02X, %02X}", bufferSize + 3, (uint8_t)toAddress, (uint8_t)sx1231->_address, CTLbyte);

  // spi_transaction_t t = {
  //     .cmd = REG_FIFO | 0x80,
  //     .length = 32,
  //     .flags = SPI_TRANS_USE_TXDATA,
  //     .tx_data = {bufferSize + 3, (uint8_t)toAddress, (uint8_t)sx1231->_address, CTLbyte},
  //     .user = sx1231,
  // };
  // esp_err_t err = spi_device_polling_transmit(sx1231->spi, &t);

  // if (err != ESP_OK) {
  //     ESP_LOGE(TAG, "Failed to transmit header");
  //     return;
  // }
  sx1231_unselect(sx1231);
  // vTaskDelay(100);
  // sx1231_select(sx1231);

  // spi_transaction_t t2 = {
  //     .cmd = REG_FIFO | 0x80,
  //     .length = bufferSize * 8,
  //     .tx_buffer = buffer,
  //     // .flags = SPI_TRANS_USE_TXDATA,
  //     // .tx_data = {'p', 'i', 's', 's'},
  //     .user = sx1231,
  // };
  // err = spi_device_polling_transmit(sx1231->spi, &t2);
  // if (err != ESP_OK) {
  //     ESP_LOGE(TAG, "Failed to transmit payload");
  //     return;
  // }
  // // ESP_LOGI(TAG, "payload=%.*s", bufferSize, (char*)buffer);
  // sx1231_unselect(sx1231);
  // while(1) {vTaskDelay(100);}

  // no need to wait for transmit mode to be ready since its handled by the radio
  sx1231_setMode(sx1231, RF69_MODE_TX);
  while ((sx1231_readReg(sx1231, REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT) == 0x00); // wait for PacketSent
  // sx1231_setMode(sx1231, RF69_MODE_STANDBY);
  sx1231_setMode(sx1231, RF69_MODE_RX);

  // ESP_LOGI(TAG, "sent packet");
}

void sx1231_receiveBegin(SX1231_t *sx1231) {
  sx1231->DATALEN = 0;
  sx1231->SENDERID = 0;
  sx1231->TARGETID = 0;
  sx1231->PAYLOADLEN = 0;
  sx1231->ACK_REQUESTED = 0;
  sx1231->ACK_RECEIVED = 0;
  sx1231->RSSI = 0;
  if (sx1231_readReg(sx1231, REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY)
    sx1231_writeReg(sx1231, REG_PACKETCONFIG2, (sx1231_readReg(sx1231, REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  sx1231_writeReg(sx1231, REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
  sx1231_setMode(sx1231, RF69_MODE_RX);
}

// internal function - for HW/HCW only:
// enables HiPower for 18-20dBm output
// should only be used with PA1+PA2
void sx1231_setHighPowerRegs(SX1231_t *sx1231, bool enable) {
  if (!sx1231->cfg.isRFM69HW_HCW || sx1231->_powerLevel<20) enable=false;
  sx1231_writeReg(sx1231, REG_TESTPA1, enable ? 0x5D : 0x55);
  sx1231_writeReg(sx1231, REG_TESTPA2, enable ? 0x7C : 0x70);
}