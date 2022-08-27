searchNodes=[{"doc":"Root level device configuration. Versioned for introspection in the future. Contians configuration for the spect radio network. See the network document for more information.","ref":"Spect.Config.html","title":"Spect.Config","type":"module"},{"doc":"Config.id will always be zero","ref":"Spect.Config.html#t:config_id/0","title":"Spect.Config.config_id/0","type":"type"},{"doc":"Root level device configuration. Mostly a container for other records.","ref":"Spect.Config.html#t:t/0","title":"Spect.Config.t/0","type":"type"},{"doc":"Current config version. Currently unused","ref":"Spect.Config.html#t:version/0","title":"Spect.Config.version/0","type":"type"},{"doc":"Represents an effect running the device. Effects are implemented as Lua scripts internally. Mode is a bitfield described below There are 8 effect slots total Each slot may use up to or all of the 8 sections on the device. See the sections document for a description of how that works.","ref":"Spect.EffectSlot.html","title":"Spect.EffectSlot","type":"module"},{"doc":"Mode is a bitfield struct 7 6 5 4 3 2 1 0 effect enable radio sync can sync - - - - - Bits 4-8 are reserved for future use effect_enalbe - enable or disable this effect slot radio_sync - enable or disable syncing of this slot on the network can_sync - enable or disable syncing of this slot via CAN","ref":"Spect.EffectSlot.html#t:mode/0","title":"Spect.EffectSlot.mode/0","type":"type"},{"doc":"Represents a slot for a potentially running effect mode is a bitfield described above script_name is the name of the lua script that should be running","ref":"Spect.EffectSlot.html#t:t/0","title":"Spect.EffectSlot.t/0","type":"type"},{"doc":"Represents a radio network instance. A network is refered to by it's ID which is a 10 bit integer. Encryption A network can be encrypted at the radio level. This is enabled by having the key set. Packets are encrypted entirely, and the network concept is implemented on top of that, so if a packet is encrypted such that it changes the network id bits, packets will not be routed to the correct network, and also contain nonsense.","ref":"Spect.Network.html","title":"Spect.Network","type":"module"},{"doc":"Encryption key. If set, all packets received on this network will be encrypted/decryped by this key. Must be EXACTLY 16 bytes if set.","ref":"Spect.Network.html#t:key/0","title":"Spect.Network.key/0","type":"type"},{"doc":"network's ID number. 10 bit integer ranging from 1..1023","ref":"Spect.Network.html#t:network_id/0","title":"Spect.Network.network_id/0","type":"type"},{"doc":"","ref":"Spect.Network.html#t:t/0","title":"Spect.Network.t/0","type":"type"},{"doc":"Represents the current device's radio identity. This should be set once and never changed.","ref":"Spect.Network.Identity.html","title":"Spect.Network.Identity","type":"module"},{"doc":"should never change in the DB","ref":"Spect.Network.Identity.html#t:network_identity_id/0","title":"Spect.Network.Identity.network_identity_id/0","type":"type"},{"doc":"","ref":"Spect.Network.Identity.html#t:t/0","title":"Spect.Network.Identity.t/0","type":"type"},{"doc":"Represents the current device's radio leader. Leaders can be changed at runtime.","ref":"Spect.Network.Leader.html","title":"Spect.Network.Leader","type":"module"},{"doc":"current leader's node id","ref":"Spect.Network.Leader.html#t:network_leader_id/0","title":"Spect.Network.Leader.network_leader_id/0","type":"type"},{"doc":"","ref":"Spect.Network.Leader.html#t:t/0","title":"Spect.Network.Leader.t/0","type":"type"},{"doc":"","ref":"Spect.Network.Node.html","title":"Spect.Network.Node","type":"module"},{"doc":"","ref":"Spect.NetworkNode.html","title":"Spect.NetworkNode","type":"module"},{"doc":"","ref":"Spect.Section.html","title":"Spect.Section","type":"module"},{"doc":"","ref":"Spect.Section.html#changeset/2","title":"Spect.Section.changeset/2","type":"function"},{"doc":"","ref":"Spect.SlotSection.html","title":"Spect.SlotSection","type":"module"},{"doc":"","ref":"readme.html","title":"README","type":"extras"}]