// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const { SerialPort } = require('serialport')
const tableify = require('tableify')

async function listSerialPorts() {
  await SerialPort.list().then((ports, err) => {
    if(err) {
      document.getElementById('error').textContent = err.message
      return
    } else {
      document.getElementById('error').textContent = ''
    }
    console.log('ports', ports);

    if (ports.length === 0) {
      document.getElementById('error').textContent = 'No ports discovered'
    }

    tableHTML = tableify(ports)
    document.getElementById('ports').innerHTML = tableHTML
  })
}

function listPorts() {
  listSerialPorts();
  setTimeout(listPorts, 2000);
}

// Set a timeout that will check for new serialPorts every 2 seconds.
// This timeout reschedules itself.
// setTimeout(listPorts, 2000);

// listSerialPorts();

const port = new SerialPort({ path: '/dev/ttyACM0', baudRate: 115200 })

// port.write('main screen turn on', function(err) {
//   if (err) {
//     return console.log('Error on write: ', err.message)
//   }
//   console.log('message written')
// })

// Open errors will be emitted as an error event
port.on('error', function(err) {
  console.log('Error: ', err.message)
});
// port.on('readable', function () {
//   console.log('Data:', port.read())
// })

var packets = [];

// Switches the port into "flowing mode"
port.on('data', function (data) {
  var packet = {
    event: data[0],
    sender: data[1],
    target: data[2],
    ackRequest: data[3],
    isAck: data[4],
    payloadLength: data[7],
    rssi: 0,
    data: data.slice(8, 8+data[7])
  };

  // two's complement signed little integer
  // JS ints are 32bit so mask off the top 16 if negative
  var comp = (((data[6] & 0xFF) << 8) | (data[5] & 0xFF));
  packet.rssi = data[6] & (1 << 7) ? 0xFFFF0000 | comp: comp;
  console.log(packet);
  packets.push(packet);
  tableHTML = tableify(packets)
  document.getElementById('ports').innerHTML = tableHTML
})

