import Config

gpio_pin = fn reg, idx when reg in 0..3 and idx in 0..31 -> 32 * reg + idx end

# Use shoehorn to start the main application. See the shoehorn
# docs for separating out critical OTP applications such as those
# involved with firmware updates.

config :shoehorn,
  init: [:nerves_runtime, :nerves_pack, :nerves_ssh],
  app: Mix.Project.config()[:app]

config :vintage_net_wizard,
  ssid: "can-link",
  dns_name: "wifi.config"

# Nerves Runtime can enumerate hardware devices and send notifications via
# SystemRegistry. This slows down startup and not many programs make use of
# this feature.

config :nerves_runtime, :kernel, use_system_registry: false

# Erlinit can be configured without a rootfs_overlay. See
# https://github.com/nerves-project/erlinit/ for more information on
# configuring erlinit.

config :nerves,
  erlinit: [
    hostname_pattern: "can-link-%s"
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
        # Connor Desktop
        "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQCz7fgeU+5LvC0j0lbJLuIgUe1VZceEL10+3OqqNY85D89jiS9Rlshu28DZsMlWGs9bKYT8d4Ee9tqien+FufWzbRqMZZob68b8wpFXoKShTgsHOI3MbnAPW0p7wytTjGYrmgTYX+gx1+PUJrMFU5a8ixtW9TGzFzcAhAz/UBHYylBGDCr6qWI2MuYZuKOvnDsklWlaQN0n/ug41WOFgAIScVyz/OHTB04kla/ZhHypl3qvSGQrz8Y9B61jUo+ASVq8f5NixQJEK4rotJUeg5weiF8X0mheJCPcXpDNXu/pNhn4vvLTVMB0UbUCD1SGG9G7lQXfWCborYYMtdvS5x/l+4Gq2W1lE6Kn6bOzg50Fe8bISkrWd2Nr6U5Zrwlv0tJwrjQUzkXqA5ISFV6tqYOvIxDnH3HyCEuZ1nOrgW2FIQYqR44HJGiNn7Fcl5Gq1qIwH+l5gdfVTCTKpKt2JCQ+wsg2GTWufe0annTtz8C70Ezl92GZ6OYD1p/djmTT8p8= connor@DESKTOP-IG22DC5",
        # Connor Laptop
        "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABgQDCBhtaHXWdtAuD6631aEBhpm8vPv90/MTQrmzg9xmokVEvyN1h+3Btsah6ADJYSC935+PrYF6wEhXLjro3LIkryDNmcfOBa0K96zwSVjgWWFhWQW52UWEO0p5awLvBCCJsII6gNMpjoRXmPD5eO9s7GWrM0peAGnugWHKy8GFnpJcHbPQgw1jghNZyUd2ZG0jiOo7bWqPCnxN8b2qyfOedjN+EkYBgDeQP7BZnhwDXrJ42IqQQb1mCWcBExiqDx0+eK17stu4YeA3+6DPsQkjcfZ7l+zlWcOyx3MNimls+zu3MyRr8gqU226BWOQ0DLyptwEMfZfaunhudyP4kHt58Cquom+woghVVc5M8Z9t/QDRtwTrDBmdfKCZy85TfORTlt5dRHSeN0esZjHTnxfa/Q3dqV3gG+sd2mz7va603/m8uq9yWej73Odpekbf7qHgojLiCZ/4zOEJDzdSg+Mg5QyYOcpWW2EEYSqaUNE6fmbRrNmXJC3JhnMQlJ4hwu6E= connor@MacBook-Pro.hsd1.co.comcast.net",
        # Connor mini-pc
        "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQCtTSUeHLIUtkYAT9Cw8e+lE8iFsVFa20AtKjXovZesQoRg2F347ivuyFXaI+91O1qi067KPn+j3jw42gdlnqX0R4DhyW0qYH69biZTQjQfq8tLT7c7VPyxOsDxXXceORnx9s0dRsy4ZiHB56/Ffz+eAzsbOEfwlwdJDkn1oiSbHSFv5HW1/agzlzV6M+nfD6As6ZIwAysw5PROfF6ikbG+UwcOAgG+d1RZDR2BTzedQrKEwYM5SiFYyqt7bQFj7BHKtkB9T4CsyU+Y1ORptFNoVyluQkaY9bTptTkj/PpWt2sntd8zKfwRHa7ysRTCWzN4XWIUWfOJsbe577ghN6Lh connor@connor-mini-pc"
      ]

# Configure the network using vintage_net
# See https://github.com/nerves-networking/vintage_net for more information
config :vintage_net,
  regulatory_domain: "US",
  config: [
    {"usb0", %{type: VintageNetDirect}},
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
  host: [:hostname, "can-link"],
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

config :nerves_time, rtc: {NervesTime.RTC.Abracon.IBO5, [bus_name: "i2c-2", address: 0x68]}

# Import target specific config. This must remain at the bottom
# of this file so it overrides the configuration defined above.
# Uncomment to use target specific configurations

# import_config "#{Mix.target()}.exs"
