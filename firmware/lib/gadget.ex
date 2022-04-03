defmodule CANLink.Gadget do

  def list_udcs do
    Path.wildcard("/sys/class/udc/*") |> Enum.map(&Path.split/1) |> Enum.map(&List.last/1)
  end

  def rm(gadget) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)

    File.write(Path.join(root, "UDC"), "")
    configs = File.ls!(Path.join([root, "configs"]))

    for config <- configs do
      for f <- File.ls!(Path.join([root, "configs", config])) do
        case f do
          "strings" -> File.rmdir(Path.join([root, "configs", config, "strings", "0x409"]))
          _ -> File.rm(Path.join([root, "configs", config, f]))
        end
      end

      File.rmdir(Path.join([root, "configs", config]))
    end

    functions = File.ls!(Path.join([root, "functions"]))
    for function <- functions do
      for f <- File.ls!(Path.join([root, "functions", function])) do
        case f do
          "strings" -> File.rmdir(Path.join([root, "functions", function, "strings", "0x409"]))
          _ -> File.rm(Path.join([root, "functions", function, f]))
        end
      end

      File.rmdir(Path.join([root, "functions", function]))
    end

    File.rmdir(Path.join([root, "strings/0x409"]))
    File.rmdir(root)
  end

  def create(gadget) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.mkdir(root)
    File.write(Path.join([root, "idVendor"]), "0x1d6b")  # Linux Foundation
    File.write(Path.join([root, "idProduct"]), "0x0104") # Multifunction Composite Gadget
    File.write(Path.join([root, "bcdDevice"]), "0x0100") # v1.0.0
    File.write(Path.join([root, "bcdUSB"]), "0x0200")    # USB 2.0


    File.write(Path.join([root, "bDeviceClass"]), "0xEF")
    File.write(Path.join([root, "bDeviceSubClass"]), "0x02")
    File.write(Path.join([root, "bDeviceProtocol"]), "0x01")

    File.mkdir_p(Path.join([root, "strings", "0x409"]))

    serialnumber = :os.cmd('boardid -b bbb -n 16') |> to_string() |> String.trim
    File.write(Path.join([root, "strings/0x409/serialnumber"]), serialnumber)
    File.write(Path.join([root, "strings/0x409/manufacturer"]), "Cone")
    File.write(Path.join([root, "strings/0x409/product"]), "CAN RGB Controller")
  end

  def acm(gadget, config) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.mkdir_p(Path.join([root, "functions/acm.usb0"]))
    File.mkdir_p(Path.join([root, "configs", config]))
    File.write(Path.join([root, "configs", config, "MaxPower"]), "250")
    File.ln_s(Path.join([root, "functions/acm.usb0"]), Path.join([root, "configs", config, "acm.usb0"]))
  end

  def rndis(gadget, config) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.mkdir_p(Path.join([root, "functions/rndis.usb0"]))
    File.mkdir_p(Path.join([root, "configs", config]))
    File.write(Path.join([root, "configs", config, "MaxPower"]), "250")
    File.ln_s(Path.join([root, "functions/rndis.usb0"]), Path.join([root, "configs", config, "rndis.usb0"]))

    File.write(Path.join([root, "os_desc/use"]), "1")
    File.write(Path.join([root, "os_desc/b_vendor_code"]), "0xcd")
    File.write(Path.join([root, "os_desc/qw_sign"]), "MSFT100")

    File.write( Path.join([root, "functions/rndis.usb0/os_desc/interface.rndis/compatible_id"]), "RNDIS")
    File.write(Path.join([root, "functions/rndis.usb0/os_desc/interface.rndis/sub_compatible_id"]), "5162001")

    File.ln_s(Path.join([root, "configs", config]), Path.join([root, "os_desc", config]))
  end

  def ecm(gadget, config) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.mkdir_p(Path.join([root, "functions/ecm.usb0"]))
    File.mkdir_p(Path.join([root, "configs", config]))
    File.write(Path.join([root, "configs", config, "MaxPower"]), "250")
    File.ln_s(Path.join([root, "functions/ecm.usb0"]), Path.join([root, "configs", config, "ecm.usb0"]))
  end

  def mass_storage(gadget, config) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.mkdir_p(Path.join([root, "configs", config]))
    File.write(Path.join([root, "configs", config, "MaxPower"]), "250")

    File.mkdir_p(Path.join([root, "functions", "mass_storage.0"]))
    File.mkdir_p(Path.join([root, "functions", "mass_storage.0", "lun.0"]))
    File.write(Path.join([root, "functions", "mass_storage.0", "lun.0"]), "/dev/mmcblk0p4")

    File.ln_s(Path.join([root, "functions/mass_storage.0"]), Path.join([root, "configs", config, "mass_storage.0"]))
  end

  def enable(gadget, port) do
    if port not in list_udcs() do
      :error
    else
      root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
      File.write(Path.join(root, "UDC"), port)
    end
  end

  def disable(gadget) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)
    File.write(Path.join(root, "UDC"), "")
  end

end
