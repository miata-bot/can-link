# Settingup a development environment

This should be done on Linux. if you're not using Linux,
get rekt i guess.

* clone this repo: `git clone git@github.com:miata-bot/can-link.git`
* clone submodules:  `git submodule --init --recursive`
* install [esp idf](https://docs.espressif.com/projects/esp-idf/en/v4.4.2/esp32s3/get-started/index.html)
* apply patches to sqlite3: `cd firmware/spect/components/esp-idf-sqlite3 && git patch am ../*.patch`
* install [asdf](https://asdf-vm.com/)
* install erlang and elixir: `cd interface/spect && asdf install`
* fetch dependencies: `cd interface/spect && mix deps.get`
* run migrations: `cd interface/spect && mix ecto.migrate`
* build firmware: `cd firmware/spect && idf.py build`

If and only of these stepes succeed should you procede.
Return to the [README](README.md)
