import Config

# Use shoehorn to start the main application. See the shoehorn
# docs for separating out critical OTP applications such as those
# involved with firmware updates.

config :shoehorn,
  init: [:nerves_runtime, :nerves_pack, :nerves_ssh],
  app: Mix.Project.config()[:app]

config :vintage_net_wizard,
  ssid: "haltech-link",
  dns_name: "hello_wifi.config"

# Nerves Runtime can enumerate hardware devices and send notifications via
# SystemRegistry. This slows down startup and not many programs make use of
# this feature.

config :nerves_runtime, :kernel, use_system_registry: false

# Erlinit can be configured without a rootfs_overlay. See
# https://github.com/nerves-project/erlinit/ for more information on
# configuring erlinit.

config :nerves,
  erlinit: [
    hostname_pattern: "haltech-link-%s"
  ]

# Configure the device for SSH IEx prompt access and firmware updates
#
# * See https://hexdocs.pm/nerves_ssh/readme.html for general SSH configuration
# * See https://hexdocs.pm/ssh_subsystem_fwup/readme.html for firmware updates

keys =
  [
    Path.join([System.user_home!(), ".ssh", "id_rsa.pub"]),
    Path.join([System.user_home!(), ".ssh", "id_ecdsa.pub"]),
    Path.join([System.user_home!(), ".ssh", "id_ed25519.pub"])
  ]
  |> Enum.filter(&File.exists?/1)

if keys == [],
  do:
    Mix.raise("""
    No SSH public keys found in ~/.ssh. An ssh authorized key is needed to
    log into the Nerves device and update firmware on it using ssh.
    See your project's config.exs for this error message.
    """)

config :nerves_ssh,
  authorized_keys:
    Enum.map(keys, &File.read!/1) ++
      [
        "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDCBhtaHXWdtAuD6631aEBhpm8vPv90/MTQrmzg9xmokVEvyN1h+3Btsah6ADJYSC935+PrYF6wEhXLjro3LIkryDNmcfOBa0K96zwSVjgWWFhWQW52UWEO0p5awLvBCCJsII6gNMpjoRXmPD5eO9s7GWrM0peAGnugWHKy8GFnpJcHbPQgw1jghNZyUd2ZG0jiOo7bWqPCnxN8b2qyfOedjN+EkYBgDeQP7BZnhwDXrJ42IqQQb1mCWcBExiqDx0+eK17stu4YeA3+6DPsQkjcfZ7l+zlWcOyx3MNimls+zu3MyRr8gqU226BWOQ0DLyptwEMfZfaunhudyP4kHt58Cquom+woghVVc5M8Z9t/QDRtwTrDBmdfKCZy85TfORTlt5dRHSeN0esZjHTnxfa/Q3dqV3gG+sd2mz7va603/m8uq9yWej73Odpekbf7qHgojLiCZ/4zOEJDzdSg+Mg5QyYOcpWW2EEYSqaUNE6fmbRrNmXJC3JhnMQlJ4hwu6E= connor@MacBook-Pro.hsd1.co.comcast.net"
      ]

# Configure the network using vintage_net
# See https://github.com/nerves-networking/vintage_net for more information
config :vintage_net,
  regulatory_domain: "US",
  config: [
    # {"usb0", %{type: VintageNetDirect}},
    {"eth0",
     %{
       type: VintageNetEthernet,
       ipv4: %{method: :dhcp}
     }},
    {"wlan0",
     %{
       type: VintageNetWiFi,
       vintage_net_wifi: %{
         networks: [
           %{
             key_mgmt: :wpa_psk,
             ssid: "HamburgerHelper",
             psk: "Gizmos123"
           }
         ]
       },
       ipv4: %{method: :dhcp}
     }}
  ]

config :mdns_lite,
  # The `host` key specifies what hostnames mdns_lite advertises.  `:hostname`
  # advertises the device's hostname.local. For the official Nerves systems, this
  # is "nerves-<4 digit serial#>.local".  mdns_lite also advertises
  # "nerves.local" for convenience. If more than one Nerves device is on the
  # network, delete "nerves" from the list.
  dns_bridge_enabled: true,
  dns_bridge_ip: {127, 0, 0, 53},
  dns_bridge_port: 53,
  dns_bridge_recursive: true,
  host: [:hostname, "haltech-link"],
  ttl: 120,

  # Advertise the following services over mDNS.
  services: [
    %{
      protocol: "ssh",
      transport: "tcp",
      port: 22
    },
    %{
      protocol: "sftp-ssh",
      transport: "tcp",
      port: 22
    },
    %{
      protocol: "epmd",
      transport: "tcp",
      port: 4369
    }
  ]

# Import target specific config. This must remain at the bottom
# of this file so it overrides the configuration defined above.
# Uncomment to use target specific configurations

# import_config "#{Mix.target()}.exs"
