# streaming_manager: open defects

Found in a review of the code at e26c75a5f. Paths are relative to `src/` unless stated.

## Security

- **DAC file upload runs a shell command built from the request.** `nginx.conf`, location
  `/upload_dac_file`: `args.file` goes unchecked into the path and into
  `os.execute("cp " .. body .. " " .. filename)`. A file name with `;` or `../` runs commands or
  writes outside `/home/redpitaya/streaming_files/dac`. All locations send
  `Access-Control-Allow-Origin: *`.
- **The config port listens on every interface by default.** streaming-server takes
  `--listen ADDRESS` to limit it (the web application keeps every interface: the address of its
  page is the announced one); the data ports always listen on every interface.

## Broadcast

- **`Broadcast.TheConfiguredHostCanChangeWhileAnnouncing` crashed once** (SEGFAULT) in the
  board image build of 2026-11-06 under `make -j8`; 20 runs in a row afterwards passed. A race of
  `setIPAddress` with the announcing thread is likely (`broadcast_lib/asio_broadcast_socket.cpp`).

## GPIO

- **The web page has no GPIO streaming UI.** `js/sm.js` registers `SM.change_gpio_status` for
  `SS_GPIO_STATUS`, which is not defined; the run/stop buttons send `SS_GPIO_START`, which starts
  the input only (the output plays what a network client sends), the page shows no status and has
  no GPIO settings. The memory sliders of the input and the output are there.
- **No GPIO file mode.** The settings have no pass mode, file name or format for the GPIO: the input
  goes to the network only, the output comes from the network only.
- **No GPIO in rpsa_client.** Only the client API (`GPIOStreamClient`) streams GPIO.
