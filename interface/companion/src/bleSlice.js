import {createSlice, PayloadAction} from '@reduxjs/toolkit';

const bluetoothReducer = createSlice({
  name: 'bluetooth',
  initialState: {
    scanning: false,
    availableDevices: [],
    connected: null,
    mode: null,
  },
  reducers: {
    bleScanStart: (state, action) => {
      state.scanning = true;
    },
    bleScanStop: (state, action) => {
      state.scanning = false;
    },
    blePeripheralDiscovered: (state, action) => {
      var index = state.availableDevices.findIndex((device) => {
        return action.payload.id == device.id  
      })
      if(index == -1) {
        state.availableDevices.push(action.payload)
      } else {
        state.availableDevices[index] = action.payload
      }
    },
    blePeripheralConnected: (state, action) => {
      state.connected = action.payload
    },
    blePeripheralDisconnected: (state, action) => {
      state.connected = null;
    },
    blePeripheralModeChange: (state, action) => {
      state.mode = action.payload;
    }
  },
});

export const {
    bleScanStart,
    bleScanStop,
    blePeripheralDiscovered,
    blePeripheralConnected,
    blePeripheralDisconnected,
    blePeripheralModeChange
} = bluetoothReducer.actions

export default bluetoothReducer.reducer