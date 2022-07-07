import React, {useState, useEffect,} from 'react';
import { Provider } from 'react-redux';
import { Text, View,Button } from 'react-native';
import { useColorScheme } from 'react-native';
import {
  NavigationContainer,
  DefaultTheme,
  DarkTheme,
} from '@react-navigation/native';
import { createBottomTabNavigator } from '@react-navigation/bottom-tabs';

import store from './src/store';
import HomeScreen from './src/screens/home';
import bleManager from './src/ble';
import BleManager from 'react-native-ble-manager';

import {Socket} from 'phoenix';

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


const joinChannel = () => {
  const socket = new Socket("ws://192.168.1.118:4000/socket", {})
  
  socket.onOpen(event => console.log('Connected.'))
  socket.onError(event => console.log('Cannot connect.'))
  socket.onClose(event => console.log('Goodbye.'))
  
  socket.connect()
  
  const channel = socket.channel('room:cone')
  channel.on("event", (value) => {
    console.log("got message", value);
    var color = hexToRgb(value.color);
    var payload = [];
    payload[0] = color.r;
    payload[1] = color.g;
    payload[2] = color.b;
    payload[3] = 0xff;
    BleManager.writeWithoutResponse("24:0A:C4:59:4F:52", "59462f12-9543-9999-12c8-58b459a2712d", "5c3a659e-897e-45e1-b016-007107c96df7", payload);
  });
  channel.join()
    .receive('ok', resp => {console.log("Joined channel", resp)})
    .receive('error', resp => {console.log("Error in joining", resp)})
}

function SettingsScreen() {
  return (
    <View style={{ flex: 1, justifyContent: 'center', alignItems: 'center' }}>
      <Button title="press" onPress={joinChannel} />
      <Text>Settings!</Text>
    </View>
  );
}

const Tab = createBottomTabNavigator();

function MyTabs() {
  return (
    <Tab.Navigator>
      <Tab.Screen name="Available Devices" component={HomeScreen} />
      <Tab.Screen name="Settings" component={SettingsScreen} />
    </Tab.Navigator>
  );
}

export default function App() {
  useEffect(() => {
    return bleManager.useEffect();
  }, []);

  const scheme = useColorScheme();
  return (
    <Provider store={store}>
      <NavigationContainer theme={scheme === 'dark' ? DarkTheme : DefaultTheme}>
        <MyTabs />
      </NavigationContainer>
    </Provider>
  );
}
