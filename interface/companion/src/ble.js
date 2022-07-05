import {
  Platform,
  NativeModules,
  NativeEventEmitter,
  PermissionsAndroid
} from 'react-native';
import { useSelector, useDispatch } from 'react-redux'
import BleManager from 'react-native-ble-manager';

import store from "./store";
import {bleScanStart, bleScanStop, blePeripheralDiscovered, blePeripheralConnected, blePeripheralDisconnected} from './bleSlice';

const BleManagerModule = NativeModules.BleManager;
const bleManagerEmitter = new NativeEventEmitter(BleManagerModule);

class ConeBleManager {
  constructor() {}

  connectPeripheral = (peripheral) => {
    BleManager.connect(peripheral.id).then(() => {
      setTimeout(() => {
        BleManager.retrieveServices(peripheral.id).then((peripheralData) => {
          console.log('Retrieved peripheral services', peripheralData);
          store.dispatch(blePeripheralConnected(peripheralData))
          // BleManager.readRSSI(peripheral.id).then((rssi) => {
          //   console.log('Retrieved actual RSSI value', rssi);
          // });
        });
      }, 900);
    })
    .catch((error) => {
      console.error("failed connecting to device", error);
    })
  }

  disconnectPeripheral = (peripheral) => {
    BleManager.disconnect(peripheral.id)
    .then(() =>{
      console.log("disconnected from ", peripheral.id);
    }).catch(() => console.error("failed to disconnect from ", peripheral.id));
  }

  handleDiscoverPeripheral = (peripheral) => {
    // console.log('Got ble peripheral', peripheral);
    store.dispatch(blePeripheralDiscovered(peripheral))
  }

  handleStopScan = () => {
    console.log('Scan is stopped');
    store.dispatch(bleScanStop())
  }

  handleUpdateValueForCharacteristic = (data) => {
    console.log('Received data from ' + data.peripheral + ' characteristic ' + data.characteristic, data.value);
  }

  handleDisconnectedPeripheral = (data) => {
    console.log('Disconnected from ' + data.peripheral);
    store.dispatch(blePeripheralDisconnected(data.peripheral))
  }

  useEffect = () => {
    BleManager.start({ showAlert: false }).then(() => {
      // Success code
      console.log("BleManager initialized");
      // store.dispatch(bleInit())
    });

    bleManagerEmitter.addListener('BleManagerDiscoverPeripheral', this.handleDiscoverPeripheral);
    bleManagerEmitter.addListener('BleManagerStopScan', this.handleStopScan);
    bleManagerEmitter.addListener('BleManagerDisconnectPeripheral', this.handleDisconnectedPeripheral);
    bleManagerEmitter.addListener('BleManagerDidUpdateValueForCharacteristic', this.handleUpdateValueForCharacteristic);

    if (Platform.OS === 'android' && Platform.Version >= 23) {
      PermissionsAndroid.check(PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION).then((result) => {
        if (result) {
          console.log("Permission is OK");
        } else {
          PermissionsAndroid.request(PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION).then((result) => {
            if (result) {
              console.log("User accept");
            } else {
              console.log("User refuse");
            }
          });
        }
      });
    }

    return (() => {
      console.log('deinit BleManager');
      bleManagerEmitter.removeListener('BleManagerDiscoverPeripheral', this.handleDiscoverPeripheral);
      bleManagerEmitter.removeListener('BleManagerStopScan', this.handleStopScan);
      bleManagerEmitter.removeListener('BleManagerDisconnectPeripheral', this.handleDisconnectedPeripheral);
      bleManagerEmitter.removeListener('BleManagerDidUpdateValueForCharacteristic', this.handleUpdateValueForCharacteristic);
    })
  }

  scanForPeripherals = () => {
    BleManager.scan(["1811"], 3, true).then((results) => {
      store.dispatch(bleScanStart())
    }).catch(err => {
      console.error(err);
      store.dispatch(bleScanStop())
    });
    BleManager.getConnectedPeripherals().then((peripherals) => {
      peripherals.forEach((peripheral) =>{ 
        console.log("connected: ", peripheral);
        store.dispatch(blePeripheralDiscovered(peripheral))
        store.dispatch(blePeripheralConnected(peripheral))
      })
    }).catch((error) => {console.error("failed to get connected peripherals", error)});
  }
}

// create singleton. when does this get executed? 
const coneBleManager = new ConeBleManager();
export default coneBleManager;