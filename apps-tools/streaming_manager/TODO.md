# streaming_manager: open defects

Found in a review of the code at e26c75a5f. Paths are relative to `src/` unless stated.

## Security

- **DAC file upload runs a shell command built from the request.** `nginx.conf`, location
  `/upload_dac_file`: `args.file` goes unchecked into the path and into
  `os.execute("cp " .. body .. " " .. filename)`. A file name with `;` or `../` runs commands or
  writes outside `/home/redpitaya/streaming_files/dac`. All locations send
  `Access-Control-Allow-Origin: *`.
- **The config port listens on every interface.** A malformed frame closes only its connection
  now (`CNetConfigManager::rejectStream`), but any host on the network may connect and change the
  settings.

## DAC

- **The DAC trigger settings have no consumer.** `CStreamSettings` keeps the DAC trigger keys,
  `CHardware::dacSetTrigger` exists, `DACStreamServer::start` does not pass them on.
- **A network DAC client that disconnects is not reported.** The playback goes on with what the
  ring holds and waits for data until a stop comes over the config port.

## GPIO

- **The web page has no GPIO streaming UI.** `js/sm.js` registers `SM.change_gpio_status` for
  `SS_GPIO_STATUS`, which is not defined; the run/stop buttons send `SS_GPIO_START`, which starts
  the input only (the output plays what a network client sends), the page shows no status and has
  no GPIO settings. The memory sliders of the input and the output are there.
- **No GPIO file mode.** The settings have no pass mode, file name or format for the GPIO: the input
  goes to the network only, the output comes from the network only.
- **No GPIO in rpsa_client.** Only the client API (`GPIOStreamClient`) streams GPIO.
- **A GPIO client that disconnects is not reported**, as with the DAC: the output keeps the lines
  of the last word (and counts the wait), the input drops its buffers until a stop comes over the
  config port.
