import 'dart:ffi';

import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import 'dart:typed_data';

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
  FlutterBluePlus flutterBlue = FlutterBluePlus.instance;
  BluetoothDevice? device = null;
  BluetoothCharacteristic? characteristic = null;
  double lat = 0.0;
  double lon = 0.0;

  IconData buttonIcon = Icons.bluetooth;

  bool scanning = false;

  void _setLatLon(event) {
    if (event.length == 16) {
      print("sub event: ");
      var a = Uint8List.fromList(event);
      setState(() {
        lat = ByteData.sublistView(a).getFloat64(0, Endian.little);
        lon = ByteData.sublistView(a).getFloat64(8, Endian.little);
        print(lat);
        print(lon);
      });
    } else {
      print("unknown payload data");
      print(event.toString());
    }
  }

  void _disconnectDevice() {
    if (device == null) return;
    print("disconnecting");

    device?.disconnect();
    flutterBlue.stopScan();

    setState(() {
      scanning = false;
      device = null;
      characteristic = null;
    });
  }

  void _connectDevice() {
    print("connecting");

    if (device == null) return;
    device?.connect(timeout: Duration(seconds: 10)).then((value) {
      print("connected");
      device?.discoverServices().then((services) {
        for (BluetoothService service in services) {
          for (BluetoothCharacteristic char in service.characteristics) {
            if (char.uuid == Guid("1899b193-de16-de23-4ebd-a5447327b12f")) {
              characteristic = char;
              setState(() {
                buttonIcon = Icons.bluetooth_connected;
              });
              char.setNotifyValue(true);
              char.value.listen((event) {
                _setLatLon(event);
              });
            } else {
              print("wrong char");
            }
          }
        }

        services.forEach((element) {
          print(element.toString());
        });
      }, onError: (error) {
        print("failed to discover services");
        print(error.toString());
      });
    }, onError: (error) {
      print("failed to connect");
      print(error.toString());
    });
  }

  void _toggleScan() {
    if (device != null) return;
    if (characteristic != null) return;

    print("starting scan");

    if (!scanning) {
      print("starting scan");
      flutterBlue.startScan(timeout: Duration(seconds: 5));
    } else {
      print("stopping scan");
      flutterBlue.stopScan();
    }
    setState(() {
      scanning = !scanning;
      if (scanning) {
        buttonIcon = Icons.bluetooth_searching;
      } else {
        buttonIcon = Icons.bluetooth;
      }
    });
    return;
  }

  @override
  Widget build(BuildContext context) {
    // This method is rerun every time setState is called, for instance as done
    // by the _toggleScan method above.
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
            ButtonBar(alignment: MainAxisAlignment.start, children: [
              TextButton(onPressed: _connectDevice, child: Text("Connect")),
              TextButton(
                  onPressed: _disconnectDevice, child: Text("Disconnect"))
            ]),
            StreamBuilder(
                stream: flutterBlue.scanResults,
                builder: (context, snapshot) {
                  if (snapshot.data == null) {
                    return Text("scan first");
                  }
                  for (ScanResult r in snapshot.data as List<ScanResult>) {
                    if (r.device.id == DeviceIdentifier("98:84:E3:E0:D9:03")) {
                      // flutterBlue.stopScan();
                      device = r.device;
                      // scanning = false;
                      // buttonIcon = Icons.bluetooth;
                      return Text("Found device: ${r.device.name}");
                    } else {
                      print(" nop ${r.device.id} ${r.device.name}");
                    }
                  }

                  if (device != null) {
                    return Text("Connected");
                  } else {
                    return Text("Scanning...");
                  }
                }),
            Container(
              width: 600,
              height: 400,
              child: FlutterMap(
                key: UniqueKey(),
                options: MapOptions(
                  center: LatLng(lat, lon),
                  zoom: 17.6,
                ),
                layers: [
                  TileLayerOptions(
                      urlTemplate:
                          'https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png',
                      subdomains: ['a', 'b', 'c']),
                  MarkerLayerOptions(
                    markers: [
                      Marker(
                        width: 80.0,
                        height: 80.0,
                        point: LatLng(39.918912845, -105.052755576),
                        builder: (ctx) => Container(
                          child: Icon(
                            Icons.car_repair,
                            color: Colors.red,
                            size: 30.0,
                          ),
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            )
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
          onPressed: _toggleScan,
          tooltip: 'Start/Stop Scan',
          child: Icon(
              buttonIcon)), // This trailing comma makes auto-formatting nicer for build methods.
    );
  }
}
