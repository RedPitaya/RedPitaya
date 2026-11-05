# streaming_manager: open defects

Found in a review of the code at e26c75a5f. Paths are relative to `src/` unless stated.

## Security

- **DAC file upload runs a shell command built from the request.** `nginx.conf`, location
  `/upload_dac_file`: `args.file` goes unchecked into the path and into
  `os.execute("cp " .. body .. " " .. filename)`. A file name with `;` or `../` runs commands or
  writes outside `/home/redpitaya/streaming_files/dac`. All locations send
  `Access-Control-Allow-Origin: *`.
- **The web application deletes any file it is given.** `web_server/src/main.cpp`: `SS_DAC_FILE`
  is not checked, `SS_DAC_FILE_CTR=2` removes it; the setting `file_sd` accepts `../` as well
  (`common_lib/settings_lib/stream_settings.cpp`).
- **A malformed frame on the config port ends the server.** `common_lib/config_net_lib/net_config_manager.cpp`
  throws `std::runtime_error` on a broken header or segment size on the asio thread, which ends
  the process; the tests in `config_net_lib/tests/test_net_robustness.cpp` expect it. The
  listener binds every interface, the segment size has no upper limit.
- **`block_size` is not checked.** `CStreamSettings::setMemoryBlockSize` takes any value; the
  memory manager adds the 128 byte header in 32 bits, so `0xFFFFFF80` becomes 0 and
  `m_ramSize / getMemoryBlockSize()` divides by zero (`common_lib/uio_lib/memory_manager_static.cpp`).
  The value comes over the network.

## DAC

- **The DAC trigger settings have no consumer.** `CStreamSettings` keeps the DAC trigger keys,
  `CHardware::dacSetTrigger` exists, `DACStreamServer::start` does not pass them on.
- **A network DAC client that disconnects is not reported.** The playback goes on with what the
  ring holds and waits for data until a stop comes over the config port.

## GPIO

- **The web page has no GPIO streaming UI.** `js/sm.js` registers `SM.change_gpio_status` for
  `SS_GPIO_STATUS`, which is not defined; the run/stop buttons send `SS_GPIO_START`, now served by
  `GPIOStreamServer`, but the page shows no status and has no GPIO settings.
- **No GPIO file mode.** The settings have no pass mode, file name or format for the GPIO: the input
  goes to the network only, the output comes from the network only.
- **No GPIO in rpsa_client.** Only the client API (`GPIOStreamClient`) streams GPIO.
- **A GPIO client that disconnects is not reported**, as with the DAC: the output keeps the lines
  of the last word, the input drops its buffers until a stop comes over the config port.

## Configuration

- **Every full configuration stops the streams.** `ServerNetConfigManager` signals
  `memoryBlockSizeChangeNofiy` for every full configuration and every load from the file, and for
  `block_size` even when its value does not change; both servers then stop ADC and DAC
  (`MEM_MODIFY`) and reallocate the memory. Signal it only when the block size changes.
