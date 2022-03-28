defmodule CANLinkl.Gadget do
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

end
