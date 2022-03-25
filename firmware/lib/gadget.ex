defmodule Gadget do
  def rm(gadget, configs, functions) do
    root = Path.join("/sys/kernel/config/usb_gadget/", gadget)

    File.write(Path.join(root, "UDC"), "")

    for config <- configs do
      for f <- File.ls!(Path.join([root, "configs", config])) do
        case f do
          "strings" -> File.rmdir(Path.join([root, "configs", config, "strings", "0x409"]))
          _ -> File.rm(Path.join([root, "configs", config, f]))
        end
      end

      File.rmdir(Path.join([root, "configs", config]))
    end

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

  def recurse_d(root, dir) do
    for d <- File.ls!(Path.join(root, dir)) do
      if File.dir?(Path.join(root, d)) do
        recurse_d(root, d)
      else
        File.rm!(Path.join(root, dir))
      end
    end
  end
end
