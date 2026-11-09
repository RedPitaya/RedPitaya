# streaming_manager: open defects

Found in a review of the code at e26c75a5f. Paths are relative to `src/` unless stated.

## Security

- **All locations of `nginx.conf` send `Access-Control-Allow-Origin: *`.**
- **The ports listen on every interface by default.** streaming-server takes `--listen ADDRESS`
  for the config and the data ports; the web application keeps every interface (the address of
  its page is the announced one).

## GPIO

- **The web page has no GPIO streaming UI.** `js/sm.js` registers `SM.change_gpio_status` for
  `SS_GPIO_STATUS`, which is not defined; the run/stop buttons send `SS_GPIO_START`, which starts
  the input only (the output plays what a network client sends), the page shows no status and has
  no GPIO settings. The memory sliders of the input and the output are there.
- **No GPIO in rpsa_client.** Only the client API (`GPIOStreamClient`) streams GPIO.
