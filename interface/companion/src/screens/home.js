import * as React from 'react';
import { Text, View, Button, FlatList, TouchableHighlight, StyleSheet, TouchableOpacity } from 'react-native';
import {
  Colors,
} from 'react-native/Libraries/NewAppScreen';

import { decrement, increment } from '../counterSlice';
import { useSelector, useDispatch } from 'react-redux'
import bleManager from '../ble';
// fixme:
import BleManager from 'react-native-ble-manager';

import { TriangleColorPicker, toHsv, fromHsv } from 'react-native-color-picker'

const action = type => store.dispatch({ type })

function hexToRgb(hex) {
  // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
  var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
  hex = hex.replace(shorthandRegex, function(m, r, g, b) {
    return r + r + g + g + b + b;
  });

  var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
  return result ? {
    r: parseInt(result[1], 16),
    g: parseInt(result[2], 16),
    b: parseInt(result[3], 16)
  } : null;
}

export const DeviceListComponent = () => {
  const peripherals = useSelector((state) => state.bluetooth.availableDevices)
  const connected = useSelector((state) => state.bluetooth.connected)
  const dispatch = useDispatch();
  const color = connected ? 'green' : 'red'
  return <>
    <FlatList
      style={{ flex: 1 }}
      data={peripherals}
      renderItem={({ item }) =>
        <TouchableOpacity onPress={ () => bleManager.connectPeripheral(item)} key={item.id} >
          <View style={[styles.row, { backgroundColor: color }]}>
            <Text style={{ fontSize: 12, textAlign: 'center', color: '#333333', padding: 10 }}>{item.name}</Text>
            <Text style={{ fontSize: 10, textAlign: 'center', color: '#333333', padding: 2 }}>RSSI: {item.rssi}</Text>
            <Text style={{ fontSize: 8, textAlign: 'center', color: '#333333', padding: 2, paddingBottom: 20 }}>{item.id}</Text>
          </View>
        </TouchableOpacity>
      }
    />
  </>
}

export const HomeScreen = () => {
  const scanning = useSelector((state) => state.bluetooth.scanning)
  const connected = useSelector((state) => state.bluetooth.connected)

  var title = 'Scan'
  if(connected)
    title = "Disconnect";

  var disabled = scanning;
  if(connected)
    disabled = false;

  return (
    <View style={{ flex: 1 }}>
      <Button
        disabled={disabled}
        title={title}
        onPress={() => {
          if(connected) {
            bleManager.disconnectPeripheral(connected);
          } else {
            bleManager.scanForPeripherals()
          }
        }} />

      <DeviceListComponent />
      {/* <TriangleColorPicker
        onColorSelected={color => {
          // console.log("color selected", color)
          // console.log("connected=", connected.characteristics);
          var color = hexToRgb(color);
          var payload = [];
          payload[0] = color.r;
          payload[1] = color.g;
          payload[2] = color.b;
          payload[3] = 0xff;
          if(connected)
            BleManager.writeWithoutResponse(connected.id, "59462f12-9543-9999-12c8-58b459a2712d", "5c3a659e-897e-45e1-b016-007107c96df7", payload);
        }}
        style={{flex: 1}}
      /> */}
    </View>
  );
}

const styles = StyleSheet.create({
  scrollView: {
    backgroundColor: Colors.lighter,
  },
  engine: {
    position: 'absolute',
    right: 0,
  },
  body: {
    backgroundColor: Colors.white,
  },
  sectionContainer: {
    marginTop: 32,
    paddingHorizontal: 24,
  },
  sectionTitle: {
    fontSize: 24,
    fontWeight: '600',
    color: Colors.black,
  },
  sectionDescription: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
    color: Colors.dark,
  },
  highlight: {
    fontWeight: '700',
  },
  footer: {
    color: Colors.dark,
    fontSize: 12,
    fontWeight: '600',
    padding: 4,
    paddingRight: 12,
    textAlign: 'right',
  },
});
export default HomeScreen;