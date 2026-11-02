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

To be done with the DAC server class of streaming_lib (the DAC counterpart of
`ADCStreamServer`).

- **A stop before the worker thread starts is lost.** `CDACStreamingApplication::genWorker` sets
  `m_GenThreadRun = true` inside the thread, `CDACStreamingManager::threadFunc` sets
  `m_isThreadRun = true` inside its thread: a `stop()` that comes first is overwritten and its
  `join()` waits for the end of the file or for ever. `m_GenThreadRun` is not initialised in the
  constructor.
- **A pack goes back to the ring while the DMA still reads it.**
  `common_lib/dac_streaming_lib/dac_streaming_application.cpp`: `unlockBuffer()` right after
  `dacSetBuffer()`. The FPGA holds two buffers (one playing, one queued); with a full ring the
  writer's next slot is the pack just handed over. Release pack k-2 when pack k is handed over,
  the rest after the end.
- **A detached DAC stop can end the next session.** `stopDACNonBlocking` in
  `streaming-server/src/dac_streaming.cpp` and `web_server/src/main.cpp` (the fix of
  `ADCStreamServer`, a session counter, applies). The globals `g_dac_app` / `g_dac_manger` of the
  console server have no mutex.

## Configuration

- **Every full configuration stops the streams.** `ServerNetConfigManager` signals
  `memoryBlockSizeChangeNofiy` for every full configuration and every load from the file, and for
  `block_size` even when its value does not change; both servers then stop ADC and DAC
  (`MEM_MODIFY`) and reallocate the memory. Signal it only when the block size changes.
