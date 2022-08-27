# BLE Characteristics and Services

## Effect Slot

## Slot

## NodeInfo

Service UUID: ``

| characteristic | name | description |
| - | - | - |
| `` | node list | subscribe then write anything to get a list of nodes |

### Node List

when subscribed to this characteristic, the following payload will be
delievered per each node this device knows about:

| 0..11 | 11..15 | 15-31 |
| node id | reserved | network id |
