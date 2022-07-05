import {call, put, take, takeEvery, all} from 'redux-saga/effects';

import coneBleManager from './ble';

export function* bleScan() {
  coneBleManager.scanForPeripherals()
}

export default function* bleSaga() {
  yield all([
  ])
}