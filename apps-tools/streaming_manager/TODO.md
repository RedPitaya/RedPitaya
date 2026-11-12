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

## rp_sdk: doubles sent to the page lose digits (shared by every web application)

`numberToString` (`Bazaar/nginx/ngx_ext_modules/ws_server/rp_sdk/rp_json.cpp`) prints a non-integer
double with `%Lf`: 6 digits after the point. Checked 2026-11-11 with the module's own `rp_json.cpp`:
0.000244140625 -> 0.000244, 1.5e-6 -> 0.000002, 1e-7 -> 0, 123456.789012345 -> 123456.789012.
Integers are exact both ways up to 2^53 (`%llu`/`%lld`, rapidjson), so the 64 bit sample limits in
`CDoubleParameter` (`SS_SAMPLES`, `SS_GPIO_IN_SAMPLES`) are safe; values from the page are parsed
exactly. Here it touches the trigger volts only (1 uV steps, below an ADC code).
Fix: the shortest round trip text (`std::to_chars`), `null` for inf/NaN instead of the undefined
casts to (unsigned) long long; rebuild `librp_sdk.a` and every module linking it; a check of a
fractional value through the page in `smui.py`.
