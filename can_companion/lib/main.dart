import 'package:flutter/material.dart';
import 'package:flutter_reactive_ble/flutter_reactive_ble.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        // This is the theme of your application.
        //
        // Try running your application with "flutter run". You'll see the
        // application has a blue toolbar. Then, without quitting the app, try
        // changing the primarySwatch below to Colors.green and then invoke
        // "hot reload" (press "r" in the console where you ran "flutter run",
        // or simply save your changes to "hot reload" in a Flutter IDE).
        // Notice that the counter didn't reset back to zero; the application
        // is not restarted.
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key? key, required this.title}) : super(key: key);

  // This widget is the home page of your application. It is stateful, meaning
  // that it has a State object (defined below) that contains fields that affect
  // how it looks.

  // This class is the configuration for the state. It holds the values (in this
  // case the title) provided by the parent (in this case the App widget) and
  // used by the build method of the State. Fields in a Widget subclass are
  // always marked "final".

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;
  final flutterReactiveBle = FlutterReactiveBle();

  void _incrementCounter() {
    setState(() {
      // This call to setState tells the Flutter framework that something has
      // changed in this State, which causes it to rerun the build method below
      // so that the display can reflect the updated values. If we changed
      // _counter without calling setState(), then the build method would not be
      // called again, and so nothing would appear to happen.
      _counter++;
      flutterReactiveBle.scanForDevices(withServices: []).listen((device) {
        print("found device");
        print(device);
      }, onError: (error) {
        print("error" + error.toString());
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _incrementCounter method above.
    //
    // The Flutter framework has been optimized to make rerunning build methods
    // fast, so that you can just rebuild anything that needs updating rather
    // than having to individually change instances of widgets.
    return Scaffold(
      appBar: AppBar(
        // Here we take the value from the MyHomePage object that was created by
        // the App.build method, and use it to set our appbar title.
        title: Text(widget.title),
      ),
      body: Center(
        // Center is a layout widget. It takes a single child and positions it
        // in the middle of the parent.
        child: Column(
          // Column is also a layout widget. It takes a list of children and
          // arranges them vertically. By default, it sizes itself to fit its
          // children horizontally, and tries to be as tall as its parent.
          //
          // Invoke "debug painting" (press "p" in the console, choose the
          // "Toggle Debug Paint" action from the Flutter Inspector in Android
          // Studio, or the "Toggle Debug Paint" command in Visual Studio Code)
          // to see the wireframe for each widget.
          //
          // Column has various properties to control how it sizes itself and
          // how it positions its children. Here we use mainAxisAlignment to
          // center the children vertically; the main axis here is the vertical
          // axis because Columns are vertical (the cross axis would be
          // horizontal).
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text(
              'You have pushed the button this many times:',
            ),
            // StreamBuilder(
            //     stream: flutterReactiveBle.scanForDevices(
            //         withServices: [],
            //         scanMode: ScanMode.lowLatency,
            //         requireLocationServicesEnabled: false),
            //     initialData: [],
            //     builder: (ctx, snapshot) {
            //       if (snapshot.hasError) {
            //         return new Text("Error!");
            //       } else if (snapshot.data == null) {
            //         return new Text("no data");
            //       }
            //       var data = snapshot.data as DiscoveredDevice;
            //       if (data.name == 'rp-080c') {
            //         print(data.name);
            //         print(data.id);
            //         return Text(data.id + data.name);
            //       } else {
            //         return Text("waiting");
            //       }
            //     }),
            StreamBuilder(
              stream: flutterReactiveBle.connectToAdvertisingDevice(
                  id: "AA:AA:AA:AA:AA:AA",
                  withServices: [],
                  prescanDuration: Duration(seconds: 5)),
              builder: (context, snapshot) {
                if (snapshot.data.runtimeType != ConnectionStateUpdate) {
                  print("non data");
                  print(snapshot.data);
                  flutterReactiveBle.scanForDevices(withServices: [
                    // Uuid.parse("7513F64B-7077-2005-F0F1-1F8664DD00FB")
                  ]).listen((device) {
                    print("found device");
                    print(device);
                  }, onError: (error) {
                    print("error" + error.toString());
                  });

                  return Text("oops");
                }

                var update = snapshot.data as ConnectionStateUpdate;
                if (update.connectionState == DeviceConnectionState.connected) {
                  print("connected");
                  final characteristic = QualifiedCharacteristic(
                      serviceId:
                          Uuid.parse("7513F64B-7077-2005-F0F1-1F8664DD00FB"),
                      characteristicId:
                          Uuid.parse("1899B193-DE16-DE23-4EBD-A5447327B12F"),
                      deviceId: update.deviceId);
                  flutterReactiveBle
                      .subscribeToCharacteristic(characteristic)
                      .listen((data) {
                    // code to handle incoming data
                    print("data" + data.toString());
                  }, onError: (dynamic error) {
                    // code to handle errors
                    print("error" + error.toString());
                  });
                  return Text("ok");

                  // return StreamBuilder(
                  //     stream: flutterReactiveBle
                  //         .discoverServices(update.deviceId)
                  //         .asStream(),
                  //     builder: (context, snapshot) {
                  //       print("discover services");
                  //       final characteristic = QualifiedCharacteristic(
                  //           serviceId: Uuid.parse(
                  //               "7513F64B-7077-2005-F0F1-1F8664DD00FB"),
                  //           characteristicId: Uuid.parse(
                  //               "1899B193-DE16-DE23-4EBD-A5447327B12F"),
                  //           deviceId: update.deviceId);
                  //       return StreamBuilder(
                  //         stream: flutterReactiveBle
                  //             .subscribeToCharacteristic(characteristic),
                  //         builder: (context, snapshot) {
                  //           print(snapshot);
                  //           return Text("subscrubed");
                  //         },
                  //       );
                  //     });
                } else {
                  return Text("waiting for connectin");
                }
              },
            ),
            Text(
              '$_counter',
              style: Theme.of(context).textTheme.headline4,
            ),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: _incrementCounter,
        tooltip: 'Increment',
        child: Icon(Icons.add),
      ), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}

// import 'package:flutter/material.dart';
// import 'package:flutter_map/flutter_map.dart';
// import 'package:latlong2/latlong.dart';
// import 'dart:developer';

// void main() {
//   runApp(TestApp());
// }

// class TestApp extends StatefulWidget {
//   @override
//   _TestAppState createState() => _TestAppState();
// }

// class _TestAppState extends State<TestApp> {
//   @override
//   void initState() {
//     super.initState();
//   }

//   @override
//   Widget build(BuildContext context) {
//     return MaterialApp(
//       home: Scaffold(
//         body: Center(
//           child: Container(
//             width: 600,
//             height: 600,
//             child: FlutterMap(
//               options: MapOptions(
//                 center: LatLng(39.918912845, -105.052755576),
//                 zoom: 17.5,
//               ),
//               layers: [
//                 TileLayerOptions(
//                     urlTemplate:
//                         'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',
//                     subdomains: ['a', 'b', 'c']),
//                 MarkerLayerOptions(
//                   markers: [
//                     Marker(
//                       width: 80.0,
//                       height: 80.0,
//                       point: LatLng(39.918912845, -105.052755576),
//                       builder: (ctx) => Container(
//                         child: Icon(
//                           Icons.car_repair,
//                           color: Colors.red,
//                           size: 30.0,
//                         ),
//                       ),
//                     ),
//                   ],
//                 ),
//               ],
//             ),
//           ),
//         ),
//       ),
//     );
//   }
// }

// Copyright 2017, Paul DeMarco.
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// import 'dart:async';
// import 'dart:math';

// import 'package:flutter/material.dart';
// import 'package:flutter_blue_plus/flutter_blue_plus.dart';
// import 'widgets.dart';
// import 'dart:developer' as developer;

// void main() {
//   runApp(FlutterBlueApp());
// }

// class FlutterBlueApp extends StatelessWidget {
//   @override
//   Widget build(BuildContext context) {
//     return MaterialApp(
//       color: Colors.lightBlue,
//       home: StreamBuilder<BluetoothState>(
//           stream: FlutterBluePlus.instance.state,
//           initialData: BluetoothState.unknown,
//           builder: (c, snapshot) {
//             final state = snapshot.data;
//             if (state == BluetoothState.on) {
//               return FindDevicesScreen();
//             }
//             return BluetoothOffScreen(state: state);
//           }),
//     );
//   }
// }

// class BluetoothOffScreen extends StatelessWidget {
//   const BluetoothOffScreen({Key? key, this.state}) : super(key: key);

//   final BluetoothState? state;

//   @override
//   Widget build(BuildContext context) {
//     return Scaffold(
//       backgroundColor: Colors.lightBlue,
//       body: Center(
//         child: Column(
//           mainAxisSize: MainAxisSize.min,
//           children: <Widget>[
//             Icon(
//               Icons.bluetooth_disabled,
//               size: 200.0,
//               color: Colors.white54,
//             ),
//             Text(
//               'Bluetooth Adapter is ${state != null ? state.toString().substring(15) : 'not available'}.',
//               style: Theme.of(context)
//                   .primaryTextTheme
//                   .subtitle1
//                   ?.copyWith(color: Colors.white),
//             ),
//           ],
//         ),
//       ),
//     );
//   }
// }

// class FindDevicesScreen extends StatelessWidget {
//   @override
//   Widget build(BuildContext context) {
//     return Scaffold(
//       appBar: AppBar(
//         title: Text('Find Devices'),
//       ),
//       body: RefreshIndicator(
//         onRefresh: () =>
//             FlutterBluePlus.instance.startScan(timeout: Duration(seconds: 4)),
//         child: SingleChildScrollView(
//           child: Column(
//             children: <Widget>[
//               StreamBuilder<List<BluetoothDevice>>(
//                 stream: Stream.periodic(Duration(seconds: 2))
//                     .asyncMap((_) => FlutterBluePlus.instance.connectedDevices),
//                 initialData: [],
//                 builder: (c, snapshot) => Column(
//                   children: snapshot.data!
//                       .map((d) => ListTile(
//                             title: Text(d.name),
//                             subtitle: Text(d.id.toString()),
//                             trailing: StreamBuilder<BluetoothDeviceState>(
//                               stream: d.state,
//                               initialData: BluetoothDeviceState.disconnected,
//                               builder: (c, snapshot) {
//                                 if (snapshot.data ==
//                                     BluetoothDeviceState.connected) {
//                                   return RaisedButton(
//                                     child: Text('OPEN'),
//                                     onPressed: () => Navigator.of(context).push(
//                                         MaterialPageRoute(
//                                             builder: (context) =>
//                                                 DeviceScreen(device: d))),
//                                   );
//                                 }
//                                 return Text(snapshot.data.toString());
//                               },
//                             ),
//                           ))
//                       .toList(),
//                 ),
//               ),
//               StreamBuilder<List<ScanResult>>(
//                 stream: FlutterBluePlus.instance.scanResults,
//                 initialData: [],
//                 builder: (c, snapshot) => Column(
//                   children: snapshot.data!
//                       .map(
//                         (r) => ScanResultTile(
//                           result: r,
//                           onTap: () => Navigator.of(context)
//                               .push(MaterialPageRoute(builder: (context) {
//                             r.device.connect();
//                             return DeviceScreen(device: r.device);
//                           })),
//                         ),
//                       )
//                       .toList(),
//                 ),
//               ),
//             ],
//           ),
//         ),
//       ),
//       floatingActionButton: StreamBuilder<bool>(
//         stream: FlutterBluePlus.instance.isScanning,
//         initialData: false,
//         builder: (c, snapshot) {
//           if (snapshot.data!) {
//             return FloatingActionButton(
//               child: Icon(Icons.stop),
//               onPressed: () => FlutterBluePlus.instance.stopScan(),
//               backgroundColor: Colors.red,
//             );
//           } else {
//             return FloatingActionButton(
//                 child: Icon(Icons.search),
//                 onPressed: () => FlutterBluePlus.instance
//                     .startScan(timeout: Duration(seconds: 4)));
//           }
//         },
//       ),
//     );
//   }
// }

// class _DeviceScreenState extends State<DeviceScreen> {
//   @override
//   Widget build(BuildContext context) {
//     return Container(color: const Color(0xFFFFE306));
//   }
// }

// class DeviceScreen extends StatefulWidget {
//   const DeviceScreen({Key? key, required this.device}) : super(key: key);
//   final BluetoothDevice device;

//   @override
//   State<DeviceScreen> createState() => _DeviceScreenState();

//   Widget _buildServiceTiles(List<BluetoothService> services) {
//     developer.log("HELP ME");
//     services.forEach((element) {
//       developer.log(element.toString());
//     });
//     var service = services.firstWhere(
//         (element) => element.uuid == "00002a28-0000-1000-8000-00805f9b34fb");

//     return Container(
//       width: 600,
//       height: 600,
//       child: FlutterMap(
//         options: MapOptions(
//           center: LatLng(39.918912845, -105.052755576),
//           zoom: 17.5,
//         ),
//         layers: [
//           TileLayerOptions(
//               urlTemplate: 'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',
//               subdomains: ['a', 'b', 'c']),
//           MarkerLayerOptions(
//             markers: [
//               Marker(
//                 width: 80.0,
//                 height: 80.0,
//                 point: LatLng(39.918912845, -105.052755576),
//                 builder: (ctx) => Container(
//                   child: Icon(
//                     Icons.car_repair,
//                     color: Colors.red,
//                     size: 30.0,
//                   ),
//                 ),
//               ),
//             ],
//           ),
//         ],
//       ),
//     );
//   }

//   @override
//   Widget build(BuildContext context) {
//     return Scaffold(
//       appBar: AppBar(
//         title: Text(device.name),
//         actions: <Widget>[
//           StreamBuilder<BluetoothDeviceState>(
//             stream: device.state,
//             initialData: BluetoothDeviceState.connecting,
//             builder: (c, snapshot) {
//               VoidCallback? onPressed;
//               String text;
//               switch (snapshot.data) {
//                 case BluetoothDeviceState.connected:
//                   onPressed = () => device.disconnect();
//                   text = 'DISCONNECT';
//                   break;
//                 case BluetoothDeviceState.disconnected:
//                   onPressed = () => device.connect();
//                   text = 'CONNECT';
//                   break;
//                 default:
//                   onPressed = null;
//                   text = snapshot.data.toString().substring(21).toUpperCase();
//                   break;
//               }
//               return FlatButton(
//                   onPressed: onPressed,
//                   child: Text(
//                     text,
//                     style: Theme.of(context)
//                         .primaryTextTheme
//                         .button
//                         ?.copyWith(color: Colors.white),
//                   ));
//             },
//           )
//         ],
//       ),
//       body: SingleChildScrollView(
//         child: Column(
//           children: <Widget>[
//             StreamBuilder<BluetoothDeviceState>(
//               stream: device.state,
//               initialData: BluetoothDeviceState.connecting,
//               builder: (c, snapshot) => ListTile(
//                 leading: (snapshot.data == BluetoothDeviceState.connected)
//                     ? Icon(Icons.bluetooth_connected)
//                     : Icon(Icons.bluetooth_disabled),
//                 title: Text(
//                     'Device is ${snapshot.data.toString().split('.')[1]}.'),
//                 subtitle: Text('${device.id}'),
//                 trailing: StreamBuilder<bool>(
//                   stream: device.isDiscoveringServices,
//                   initialData: false,
//                   builder: (c, snapshot) => IndexedStack(
//                     index: snapshot.data! ? 1 : 0,
//                     children: <Widget>[
//                       IconButton(
//                         icon: Icon(Icons.refresh),
//                         onPressed: () => device.discoverServices(),
//                       ),
//                       IconButton(
//                         icon: SizedBox(
//                           child: CircularProgressIndicator(
//                             valueColor: AlwaysStoppedAnimation(Colors.grey),
//                           ),
//                           width: 18.0,
//                           height: 18.0,
//                         ),
//                         onPressed: null,
//                       )
//                     ],
//                   ),
//                 ),
//               ),
//             ),
//             StreamBuilder<int>(
//               stream: device.mtu,
//               initialData: 0,
//               builder: (c, snapshot) => ListTile(
//                 title: Text('MTU Size'),
//                 subtitle: Text('${snapshot.data} bytes'),
//                 trailing: IconButton(
//                   icon: Icon(Icons.edit),
//                   onPressed: () => device.requestMtu(223),
//                 ),
//               ),
//             ),
//             StreamBuilder<List<BluetoothService>>(
//               stream: device.services,
//               initialData: [],
//               builder: (c, snapshot) {
//                 return Container(
//                   child: _buildServiceTiles(snapshot.data!),
//                 );
//               },
//             ),
//           ],
//         ),
//       ),
//     );
//   }
// }
