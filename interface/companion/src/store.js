import logger from 'redux-logger';

import { configureStore, getDefaultMiddleware } from '@reduxjs/toolkit'
import {useDispatch} from 'react-redux';
import createSagaMiddleware from 'redux-saga';
import {all, fork} from 'redux-saga/effects';

import counterSlice from './counterSlice'
import bluetoothReducer from './bleSlice';
import bleSaga from './bleSaga';

const sagaMiddleware = createSagaMiddleware();
const rootSaga = function* rootSaga() {
  yield all([
    fork(bleSaga)
  ])
}

export default store = configureStore({
  reducer: {
    counter: counterSlice,
    bluetooth: bluetoothReducer
  },
  middleware: (getDefaultMiddleware) => getDefaultMiddleware().concat(logger).concat(sagaMiddleware),
  devTools: true
})
sagaMiddleware.run(rootSaga)
export const useAppDispatch = () => useDispatch();