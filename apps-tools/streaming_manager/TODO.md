# streaming_manager: open defects

Found in a review of the code at e26c75a5f. Paths are relative to `src/` unless stated.

## Security

- **DAC file upload runs a shell command built from the request.** `nginx.conf`, location
  `/upload_dac_file`: `args.file` goes unchecked into the path and into
  `os.execute("cp " .. body .. " " .. filename)`. A file name with `;` or `../` runs commands or
  writes outside `/home/redpitaya/streaming_files/dac`. All locations send
  `Access-Control-Allow-Origin: *`.
- **The ports listen on every interface by default.** streaming-server takes `--listen ADDRESS`
  for the config and the data ports; the web application keeps every interface (the address of
  its page is the announced one).

## GPIO

- **The web page has no GPIO streaming UI.** `js/sm.js` registers `SM.change_gpio_status` for
  `SS_GPIO_STATUS`, which is not defined; the run/stop buttons send `SS_GPIO_START`, which starts
  the input only (the output plays what a network client sends), the page shows no status and has
  no GPIO settings. The memory sliders of the input and the output are there.
- **The client API does not report the GPIO file mode.** The board writes the input to a file
  (`gpio_in_pass_mode=FILE`, BIN/CSV/VCD in `/home/redpitaya/streaming_files/gpio`) and plays the
  output from one (`gpio_out_pass_mode=FILE`); `ClientNetConfigManager` has the signals of its
  replies (64..72), `streaming_api_lib` (ConfigCallback, GPIOStreamClient) does not pass them on.
- **No GPIO in rpsa_client.** Only the client API (`GPIOStreamClient`) streams GPIO.

## BIN stream file

- **The client API plays WAV and TDMS only.** `DACStreamClient` has no BIN mode;
  `rpsa_client -o -f bin` and the board (`file_type_sd=BIN`) play it.
