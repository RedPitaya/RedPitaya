#!/usr/bin/python3

import streaming
import queue
import time
import numpy as np

# Signal detection threshold
SIGNAL_THRESHOLD = 1000  # Adjust for your needs

# Shared data between callbacks
g_list = queue.Queue()

# Global clients
adcClient = None
dacClient = None

# ==================== ADC Callback Class ====================
class ADCCallbackHandler(streaming.ADCCallback):
    def __init__(self):
        super().__init__()
        self.adcCounter = 0
        self.fpgaLost = 0

    def receivePack(self, client, pack):
        # Update counters
        self.adcCounter += (pack.channel1.samples + pack.channel2.samples +
                           pack.channel3.samples + pack.channel4.samples)

        self.fpgaLost += max(pack.channel1.fpgaLost, pack.channel2.fpgaLost,
                            pack.channel3.fpgaLost, pack.channel4.fpgaLost)

        # Analyze received data for signal presence (using channel1 as example)
        self.analyzeSignal(pack.channel1)

    def connected(self, client, host):
        print(f"ADC connected to {host}")

    def disconnected(self, client, host):
        print(f"ADC disconnected from {host}")

    def error(self, client, host, code):
        print(f"ADC error {host} code {code}")

    def analyzeSignal(self, channel):
        if channel.samples == 0 or not hasattr(channel, 'raw') or not channel.raw:
            return

        found = False

        # Analyze raw data for signal presence
        for sample in channel.raw:
            if abs(sample) > SIGNAL_THRESHOLD:
                found = True
                break

        # If signal detected, save data to shared buffer
        if found:
            g_list.put(channel.raw)
            print(f"Signal detected! Saved {len(channel.raw)} samples to DAC buffer")

# ==================== DAC Callback Class ====================
class DACCallbackHandler(streaming.DACCallback):
    def __init__(self):
        super().__init__()
        self.dacCounter = 0

    def streamData16Bit(self, client, ch1, ch2, size):
        try:
            # If no signal detected or buffer empty, send silence (zeros)
            if g_list.empty():
                if ch1 is not None:
                    # Create zeros array
                    zeros = np.zeros(size, dtype=np.int16)
                    # Copy to memoryview
                    memview = ch1.cast('h')
                    memview[:] = zeros[:]

                if ch2 is not None:
                    zeros = np.zeros(size, dtype=np.int16)
                    memview = ch2.cast('h')
                    memview[:] = zeros[:]

                return False  # Don't stop

            # Get data from queue
            data = g_list.get_nowait()

            # Convert to numpy array if needed
            if not isinstance(data, np.ndarray):
                data = np.array(data, dtype=np.int16)

            samplesToSend = min(size, len(data))

            # Send to channel 1
            if ch1 is not None:
                ch1_view = ch1.cast('h')
                # Copy data
                ch1_view[:samplesToSend] = data[:samplesToSend]
                # Fill remaining with zeros
                if samplesToSend < size:
                    ch1_view[samplesToSend:] = 0

            # Send to channel 2 (same data)
            if ch2 is not None:
                ch2_view = ch2.cast('h')
                ch2_view[:samplesToSend] = data[:samplesToSend]
                if samplesToSend < size:
                    ch2_view[samplesToSend:] = 0

            return False  # Return False to continue streaming

        except Exception as e:
            print(f"Error in streamData16Bit: {e}")
            return True  # Stop on error

    def streamData8Bit(self, client, ch1, ch2, size):
        try:
            # If no signal detected or buffer empty, send silence (zeros)
            if g_list.empty():
                if ch1 is not None:
                    zeros = np.zeros(size, dtype=np.int8)
                    memview = ch1.cast('b')
                    memview[:] = zeros[:]

                if ch2 is not None:
                    zeros = np.zeros(size, dtype=np.int8)
                    memview = ch2.cast('b')
                    memview[:] = zeros[:]

                return False

            data = g_list.get_nowait()

            # Convert 16-bit to 8-bit by taking the most significant byte
            if isinstance(data, np.ndarray):
                data_8bit = (data >> 8).astype(np.int8)
            else:
                data_8bit = np.array([(x >> 8) & 0xFF for x in data], dtype=np.int8)

            samplesToSend = min(size, len(data_8bit))

            if ch1 is not None:
                ch1_view = ch1.cast('b')
                ch1_view[:samplesToSend] = data_8bit[:samplesToSend]
                if samplesToSend < size:
                    ch1_view[samplesToSend:] = 0

            if ch2 is not None:
                ch2_view = ch2.cast('b')
                ch2_view[:samplesToSend] = data_8bit[:samplesToSend]
                if samplesToSend < size:
                    ch2_view[samplesToSend:] = 0

            return False

        except Exception as e:
            print(f"Error in streamData8Bit: {e}")
            return True

    def sentPack(self, client, ch1_size, ch2_size):
        self.dacCounter += 1

    def connected(self, client, host):
        print(f"DAC connected to {host}")

    def disconnected(self, client, host):
        print(f"DAC disconnected from {host}")

    def error(self, client, host, code):
        print(f"DAC error {host} code {code}")

    def stopped(self, client, host):
        print(f"DAC stopped on host: {host}")

    def stoppedFileEnd(self, client, host):
        print(f"DAC stopped - file end on host: {host}")

    def stoppedFileBroken(self, client, host):
        print(f"DAC stopped - file broken on host: {host}")

    def stoppedEmpty(self, client, host):
        print(f"DAC stopped - empty on host: {host}")

    def stoppedMissingFile(self, client, host):
        print(f"DAC stopped - missing file on host: {host}")

    def stoppedMemError(self, client, host):
        print(f"DAC stopped - memory error on host: {host}")

    def stoppedMemModify(self, client, host):
        print(f"DAC stopped - memory modified on host: {host}")

# ==================== Config Callback Class ====================
class ConfigCallbackHandler(streaming.ConfigCallback):
    def sigInt(self):
        global adcClient, dacClient
        if adcClient:
            adcClient.notifyStop()
        if dacClient:
            dacClient.notifyStop()

    def configConnected(self, client, host):
        print(f"Config client connected to {host}")

    def configError(self, client, host, code):
        print(f"Config client error on {host} code {code}")

# ==================== Main Function ====================
def main():
    global adcClient, dacClient

    # Create ADC and DAC clients
    confClient = streaming.ConfigStreamClient()
    adcClient = streaming.ADCStreamClient(confClient)
    dacClient = streaming.DACStreamClient(confClient)

    # Create separate callback handlers
    adcCallback = ADCCallbackHandler()
    dacCallback = DACCallbackHandler()
    confCallback = ConfigCallbackHandler()

    # Set callbacks for clients
    adcClient.setCallback(adcCallback)
    dacClient.setCallback(dacCallback)
    confClient.addCallback(confCallback)

    # Enable logs
    adcClient.setVerbose(True)
    dacClient.setVerbose(True)
    confClient.setVerbose(True)

    # ==================== ADC Connection ====================
    print("Connecting ADC...")
    if not confClient.connect():
        print("ADC client did not connect")
        return 1

    # ADC configuration
    confClient.sendConfig('adc_pass_mode', 'NET')
    confClient.sendConfig('adc_decimation', '64')
    confClient.sendConfig('block_size', '65536')
    confClient.sendConfig('adc_size', '1638400')
    confClient.sendConfig('channel_state_1', 'ON')
    confClient.sendConfig('channel_state_2', 'ON')

    # DAC configuration
    confClient.sendConfig('dac_pass_mode', 'DAC_NET')
    confClient.sendConfig('dac_rate', '1953125')
    confClient.sendConfig('dac_size', '1638400')

    # Get host
    hosts = confClient.getHosts()
    if not hosts:
        print("No hosts found")
        return 1
    host = hosts[0]

    # ==================== Start Streams ====================
    # Start ADC
    if adcClient.startStreaming():
        print("ADC streaming started")
    else:
        print("Error starting ADC streaming")
        return 1

    # Start DAC with 16-bit mode (better quality)
    if dacClient.startStreamingFromMemorySink(host, True, True, streaming.DAC_16BIT):
        print("DAC streaming started (16-bit mode)")
    else:
        print("Failed to start DAC streaming")
        adcClient.notifyStop()
        return 1

    print("Both ADC and DAC are running. Waiting for data...")
    print(f"Signal threshold: {SIGNAL_THRESHOLD}")

    # ==================== Wait for Completion ====================
    # Wait for ADC to finish (it stops after receiving 50M samples)
    # Note: In original C++ code, ADC stops after 50M samples, but here we wait indefinitely
    # You may need to add a stop condition similar to the first Python example
    try:
        adcClient.wait()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
        confCallback.sigInt()

    # Stop DAC
    dacClient.notifyStop()
    dacClient.wait()

    # ==================== Print Statistics ====================
    print("\n=== Statistics ===")
    print(f"ADC received samples: {adcCallback.adcCounter}")
    print(f"ADC lost samples: {adcCallback.fpgaLost}")
    print(f"DAC sent packets: {dacCallback.dacCounter}")

    return 0

if __name__ == "__main__":
    exit(main())