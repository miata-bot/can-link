#ifndef SX1231_H
#define SX1231_H

#include <stdint.h>
#include <stdbool.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <sdkconfig.h>
#include <esp_timer.h>

#define RF69_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)
#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91

#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR   0
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000
#define RF69_FSTEP  61.03515625 // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet)

// TWS: define CTLbyte bits
#define RFM69_CTL_SENDACK   0x80
#define RFM69_CTL_REQACK    0x40

#define RFM69_ACK_TIMEOUT   30  // 30ms roundtrip req for 61byte packets

/// Configurations of the SX1231 radio
typedef struct {
    spi_host_device_t host; ///< The SPI host used, set before calling `SX1231_init()`
    gpio_num_t gpio_cs;
    gpio_num_t gpio_int;
    gpio_num_t gpio_reset;

    uint8_t freqBand;
    uint16_t nodeID;
    uint8_t networkID;
    bool isRFM69HW_HCW;
} SX1231_config_t;

typedef struct SX1231 {
    // Public
    uint8_t DATA[RF69_MAX_DATA_LEN+1]; // RX/TX payload buffer, including end of string NULL char
    uint8_t DATALEN;
    uint16_t SENDERID;
    uint16_t TARGETID; // should match _address
    uint8_t PAYLOADLEN;
    uint8_t ACK_REQUESTED;
    uint8_t ACK_RECEIVED; // should be polled immediately after sending a packet with ACK request
    int16_t RSSI; // most accurate RSSI during reception (closest to the reception). RSSI of last packet.
    uint8_t _mode; // should be protected?

    // Private
    uint8_t  _slaveSelectPin;
    uint8_t  _interruptPin;
    uint8_t  _interruptNum;
    uint16_t _address;
    uint8_t  _powerLevel;
    bool     _dataAvailable;

    SX1231_config_t cfg;
    spi_device_handle_t spi;
} SX1231_t;

// Public API

esp_err_t sx1231_initialize(SX1231_config_t *sx1231, SX1231_t** out_ctx);
void sx1231_setAddress(SX1231_t *sx1231, uint16_t addr);
void sx1231_setNetwork(SX1231_t *sx1231, uint8_t networkID);
bool sx1231_canSend(SX1231_t *sx1231);
void sx1231_send(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK);
bool sx1231_sendWithRetry(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries, uint8_t retryWaitTime);
bool sx1231_receiveDone(SX1231_t *sx1231);
bool sx1231_ACKReceived(SX1231_t *sx1231, uint16_t fromNodeID);
bool sx1231_ACKRequested(SX1231_t *sx1231);
void sx1231_sendACK(SX1231_t *sx1231, const void* buffer, uint8_t bufferSize);
uint32_t sx1231_getFrequency(SX1231_t *sx1231);
void sx1231_setFrequency(SX1231_t *sx1231, uint32_t freqHz);
void sx1231_encrypt(SX1231_t *sx1231, const char* key);
void sx1231_setCS(SX1231_t *sx1231, uint8_t newSPISlaveSelect);
bool sx1231_setIrq(SX1231_t *sx1231, uint8_t newIRQPin);
int16_t sx1231_readRSSI(SX1231_t *sx1231, bool forceTrigger); // *current* signal strength indicator; e.g. < -90dBm says the frequency channel is free + ready to transmit
void sx1231_setHighPower(SX1231_t *sx1231, bool _isRFM69HW_HCW); // has to be called after initialize(SX1231_t *sx1231, ) for RFM69 HW/HCW
void sx1231_setPowerLevel(SX1231_t *sx1231, uint8_t level); // reduce/increase transmit power level
int8_t sx1231_setPowerDBm(SX1231_t *sx1231, int8_t dBm); // reduce/increase transmit power level, in dBm

uint8_t sx1231_getPowerLevel(SX1231_t *sx1231); // get powerLevel
void sx1231_sleep(SX1231_t *sx1231);
uint8_t sx1231_readTemperature(SX1231_t *sx1231, uint8_t calFactor); // get CMOS temperature (8bit)
void sx1231_rcCalibration(SX1231_t *sx1231); // calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]
void sx1231_set300KBPS(SX1231_t *sx1231);
uint8_t sx1231_setLNA(SX1231_t *sx1231, uint8_t newReg);

// allow hacking registers by making these public
uint8_t sx1231_readReg(SX1231_t *sx1231, uint8_t addr);
void sx1231_writeReg(SX1231_t *sx1231, uint8_t addr, uint8_t val);
// void sx1231_readAllRegs(SX1231_t *sx1231);
// void sx1231_readAllRegsCompact(SX1231_t *sx1231);
void sx1231_print_regs(SX1231_t* sx1231);

// ListenMode sleep/timer
void sx1231_listenModeSleep(SX1231_t *sx1231, uint16_t millisInterval);
void sx1231_endListenModeSleep(SX1231_t *sx1231);
void sx1231_setMode(SX1231_t *sx1231, uint8_t mode);
void sx1231_select(SX1231_t *sx1231);
void sx1231_unselect(SX1231_t *sx1231);

// Private API
void sx1231_isr(void* arg);
void sx1231_isr_task(void* arg);

esp_err_t sx1231_install_interrupts(SX1231_t *sx1231);
esp_err_t sx1231_reset(SX1231_t *sx1231);

void sx1231_interruptHandler(SX1231_t *sx1231);
void sx1231_sendFrame(SX1231_t *sx1231, uint16_t toAddress, const void* buffer, uint8_t size, bool requestACK, bool sendACK);

// for ListenMode sleep/timer
void sx1231_delayIrq(SX1231_t *sx1231);

void sx1231_receiveBegin(SX1231_t *sx1231);
void sx1231_setHighPowerRegs(SX1231_t *sx1231, bool enable);

#endif