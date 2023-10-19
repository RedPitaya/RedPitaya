"""SCPI access to Red Pitaya."""

import socket
import struct
import numpy as np

__author__ = "Luka Golinar, Iztok Jeras, Miha Gjura"
__copyright__ = "Copyright 2023, Red Pitaya"

class scpi (object):
    """SCPI class used to access Red Pitaya over an IP network."""
    delimiter = '\r\n'

    def __init__(self, host, timeout=None, port=5000):
        """Initialize object and open IP connection.
        Host IP should be a string in parentheses, like '192.168.1.100'.
        """
        self.host    = host
        self.port    = port
        self.timeout = timeout

        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

            if timeout is not None:
                self._socket.settimeout(timeout)

            self._socket.connect((host, port))

        except socket.error as e:
            print('SCPI >> connect({!s:s}:{:d}) failed: {!s:s}'.format(host, port, e))

    def __del__(self):
        if self._socket is not None:
            self._socket.close()
        self._socket = None

    def close(self):
        """Close IP connection."""
        self.__del__()

    def rx_txt(self, chunksize = 4096):
        """Receive text string and return it after removing the delimiter."""
        msg = ''
        while 1:
            chunk = self._socket.recv(chunksize).decode('utf-8') # Receive chunk size of 2^n preferably
            msg += chunk
            if (len(msg) >= 2 and msg[-2:] == self.delimiter):
                return msg[:-2]

    def rx_txt_check_error(self, chunksize = 4096,stop = True):
        msg = self.rx_txt(chunksize)
        self.check_error(stop)
        return msg

    def rx_arb(self):
        """ Recieve binary data from scpi server"""
        numOfBytes = 0
        data=b''
        while len(data) != 1:
            data = self._socket.recv(1)
        if data != b'#':
            return False
        data=b''

        while len(data) != 1:
            data = self._socket.recv(1)
        numOfNumBytes = int(data)
        if numOfNumBytes <= 0:
            return False
        data=b''

        while len(data) != numOfNumBytes:
            data += (self._socket.recv(1))
        numOfBytes = int(data)
        data=b''

        while len(data) < numOfBytes:
            r_size = min(numOfBytes - len(data),4096)
            data += (self._socket.recv(r_size))
        return data

    def rx_arb_check_error(self,stop = True):
        data = self.rx_arb()
        self.check_error(stop)
        return data

    def tx_txt(self, msg):
        """Send text string ending and append delimiter."""
        return self._socket.sendall((msg + self.delimiter).encode('utf-8')) # was send(().encode('utf-8'))

    def tx_txt_check_error(self, msg,stop = True):
        self.tx_txt(msg)
        self.check_error(stop)

    def txrx_txt(self, msg):
        """Send/receive text string."""
        self.tx_txt(msg)
        return self.rx_txt()

    def check_error(self,stop = True):
        res = int(self.stb_q())
        if (res & 0x4):
            while 1:
                err = self.err_n()
                if (err.startswith('0,')):
                    break
                print(err)
                n = err.split(",")
                if (len(n) > 0 and stop and int(n[0]) > 9500):
                    exit(1)

# SCPI command functions

    def sour_set(
        self,
        chan: int,
        func: str = "sine",
        volt: float = 1,
        freq: float = 1000,
        offset: float = 0,
        phase: float = 0,
        dcyc: float = 0.5,
        data: np.ndarray = None,
        burst: bool = False,
        ncyc: int = 1,
        nor: int = 1,
        period: int = None,
        trig: str = "int",
        sdrlab: bool = False,
        siglab: bool = False,
    ) -> None:

        """
        Set the parameters for signal generator on one channel.

        Parameters
        -----------
            chan (int) :
                Output channel (either 1 or 2).
            func (str, optional) :
                Waveform of the signal (SINE, SQUARE, TRIANGLE, SAWU,
                SAWD, PWM, ARBITRARY, DC, DC_NEG).
                Defaults to `sine`.
            volt (int, optional) :
                Amplitude of signal {-1, 1} Volts. {-5, 5} for SIGNALlab 250-12.
                Defaults to 1.
            freq (int, optional) :
                Frequency of signal. Not relevant if 'func' is "DC" or "DC_NEG".
                Defaults to 1000.
            offset (int, optional) :
                Signal offset {-1, 1} Volts. {-5, 5} for SIGNALlab 250-12.
                Defaults to 0.
            phase (int, optional) :
                Phase of signal {-360, 360} degrees.
                Defaults to 0.
            dcyc (float, optional) :
                Duty cycle, where 1 corresponds to 100%.
                Defaults to 0.5.
            data (ndarray, optional) :
                Numpy ``ndarray`` of max 16384 values, floats in range {-1,1}
                (or {-5,5} for SIGNALlab).
                Define the custom waveform if "func" is "ARBITRARY".
                Defaults to `None`.
            burst (bool, optional) :
                Enable/disable Burst mode. (`True` - BURST, `False` - CONINUOUS)
                Generate "nor" number of "ncyc" periods with total time "period". 
                Defaults to `False`.
            ncyc (int, optional) : 
                Number of periods in one burst.
                Defaults to 1.
            nor (int, optional) : 
                Number of repeated bursts.
                Defaults to 1.
            period (_type_, optional) :
                Total time of one burst in µs {1, 5e8}. Includes the signal and delay.
                Defaults to `None`.
            trig (str, optional):
                Trigger source (EXT_PE, EXT_NE, INT, GATED).
                Defaults to `int` (internal).
            sdrlab (bool, optional):
                `True` if operating with SDRlab 122-16.
                Defaults to `False`.
            siglab (bool, optional):
                `True` if operating with SIGNALlab 250-12.
                Defaults to `False`.

        The settings will work on any Red Pitaya board. If operating on a board
        other than STEMlab 125-14, change the bool value of the appropriate
        parameter to true (sdrlab, siglab)

        Raises
        ------

        Raises errors if the input parameters are out of range.
        
        """

        ### Constants ###
        waveform_list = ["SINE","SQUARE","TRIANGLE","SAWU","SAWD","PWM","ARBITRARY","DC","DC_NEG"]
        trigger_list = ["EXT_PE","EXT_NE","INT","GATED"]
        buff_size = 16384

        ### Limits ###
        volt_lim = 1
        offs_lim = 1
        phase_lim = 360
        freq_up_lim = 50e6          # 50 MHz
        freq_down_lim = 0

        if siglab:
            volt_lim = 5
            offs_lim = 5
        elif sdrlab:
            freq_down_lim = 300e3   # 300 kHz



        ### CHECK FOR ERRORS ###

        try:
            assert chan in (1,2)
        except AssertionError as channel_err:
            raise ValueError("Channel needs to be either 1 or 2") from channel_err

        try:
            assert func.upper() in waveform_list
        except AssertionError as waveform_err:
            raise ValueError(f"{func.upper()} is not a defined waveform") from waveform_err

        try:
            assert freq_down_lim < freq <= freq_up_lim
        except AssertionError as freq_err:
            raise ValueError(f"Frequency is out of range {freq_down_lim, freq_up_lim} Hz") from freq_err

        try:
            assert abs(volt) <= volt_lim
        except AssertionError as ampl_err:
            raise ValueError(f"Amplitude is out of range {-volt_lim, volt_lim} V") from ampl_err

        try:
            assert abs(offset) <= offs_lim
        except AssertionError as offs_err:
            raise ValueError(f"Offset is out of range {-offs_lim, offs_lim} V") from offs_err

        try:
            assert 0 <= dcyc <= 1
        except AssertionError as dcyc_err:
            raise ValueError(f"Duty Cycle is out of range {0, 1}") from dcyc_err

        try:
            assert abs(phase) <= phase_lim
        except AssertionError as phase_err:
            raise ValueError(f"Phase is out of range {-phase_lim, phase_lim} deg") from phase_err

        if data is not None:

            try:
                assert data.shape[0] <= buff_size
            except AssertionError as data_err:
                raise ValueError(f"Data array is too long. Max length is {buff_size}") from data_err

            #try:
            #    assert max(absolute(data)) <= volt_lim
            #except AssertionError:
            #    raise ValueError(f"Amplitude of data is out of range {-volt_lim, volt_lim}")

        try:
            assert ncyc >= 1
        except AssertionError as ncyc_err:
            raise ValueError("NCYC minimum is 1") from ncyc_err

        try:
            assert nor >= 1
        except AssertionError as nor_err:
            raise ValueError("NOR minimum is 1") from nor_err

        if period is not None:
            try:
                assert period >= 1
            except AssertionError as period_err:
                raise ValueError("Minimal burst period 1 µs") from period_err

        try:
            assert trig.upper() in trigger_list
        except AssertionError as trig_err:
            raise ValueError(f"{trig.upper()} is not a defined trigger source") from trig_err

        try:
            assert not((siglab is True) and (sdrlab is True))
        except AssertionError as board_err:
            raise ValueError("Please select only one board option. 'siglab' and 'sdrlab' cannot be true at the same time.") from board_err



        ### Variables ###
        wf_data = []


        ### SEND COMMANDS TO RP ###
        self.tx_txt(f"SOUR{chan}:FUNC {func.upper()}")
        self.tx_txt(f"SOUR{chan}:VOLT {volt}")

        if func.upper() not in waveform_list[7:9]:
            self.tx_txt(f"SOUR{chan}:FREQ:FIX {freq}")

        self.tx_txt(f"SOUR{chan}:VOLT:OFFS {offset}")
        self.tx_txt(f"SOUR{chan}:PHAS {phase}")

        if func.upper() == "PWM":
            self.tx_txt(f"SOUR{chan}:DCYC {dcyc}")

        if (data is not None) and (func.upper() == "ARBITRARY"):
            for n in data:
                wf_data.append(f"{n:.5f}")
            cust_wf = ", ".join(map(str, wf_data))

            self.tx_txt(f"SOUR{chan}:TRAC:DATA:DATA {cust_wf}")

        if burst:
            self.tx_txt(f"SOUR{chan}:BURS:STAT BURST")
            self.tx_txt(f"SOUR{chan}:BURS:NCYC {ncyc}")
            self.tx_txt(f"SOUR{chan}:BURS:NOR {nor}")

            if period is not None:
                self.tx_txt(f"SOUR{chan}:BURS:INT:PER {period}")
        else:
            self.tx_txt(f"SOUR{chan}:BURS:STAT CONTINUOUS")

        self.tx_txt(f"SOUR{chan}:TRIG:SOUR {trig.upper()}")

        #print(f"SOUR{chan} set successfully")

    def acq_set(
        self,
        dec: int = 1,
        trig_lvl: float = 0,
        trig_delay: int = 0,
        trig_delay_ns: bool = False,
        units: str = None,
        sample_format: str = None,
        averaging: bool = True,
        gain: list = None,               # 2 channels (double the length if 4-input)
        coupling: list = None,           # 2 channels
        ext_trig_lvl: float = 0,
        siglab: bool = False,
        input4: bool = False
    ) -> None:

        """

        Set the parameters for signal acquisition.

        Parameters
        -----------

            dec (int, optional) : 
                Decimation (1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048,
                4096, 8192, 16384, 32768, 65536)
                Defaults to 1.
            trig_lvl (float, optional) :
                Trigger level in Volts. {-1, 1} Volts on LV gain or {-20, 20} Volts on HV gain.
                Defaults to 0.
            trig_delay (int, optional) :
                Trigger delay in samples (if trig_delay_ns = True, then the delay is in ns)
                Defaults to 0.
            trig_delay_ns (bool, optional) :
                Change the trigger delay to nanoseconds instead of samples.
                Defaults to False.
            units (str, optional) :
                The units in which the acquired data will be returned.
                Defaults to "VOLTS".
            sample_format (str, optional) :
                The format in which the acquired data will be returned.
                Defaults to "ASCII".
            averaging (bool, optional) :
                Enable/disable averaging. When True, if decimation is higher than 1,
                each returned sample is the average of the taken samples. For example,
                if dec = 4, the returned sample will be the average of the 4 decimated
                samples.
                Defaults to True.
            gain (list(str), optional) :
                HV / LV - (High (1:20) or Low (1:1 attenuation)) 
                The first element in list applies to the SOUR1 and the second to SOUR2.
                Refers to jumper settings on Red Pitaya fast analog inputs.
                (1:20 and 1:1 attenuator for SIGNALlab 250-12)
                Defaults to ["LV","LV"].
            coupling (list(str), optional) :
                AC / DC - coupling mode for fast analog inputs.
                The first element in list applies to the SOUR1 and the second to SOUR2.
                (Only SIGNALlab 250-12)
                Defaults to ["DC","DC"].
            ext_trig_lvl (float, optional) :
                Set trigger external level in V.
                (Only SIGNALlab 250-12)
                Defaults to 0.
            siglab (bool, optional) :
                Set to True if operating with SIGNALlab 250-12.
                Defaults to False.
            input4 (bool, optional) :
                Set to True if operating with STEMlab 125-14 4-Input.
                Defaults to False.

        The settings will work on any Red Pitaya board. If operating on SIGNALlab 250-12
        or STEMlab 125-14 4-Input change the bool value of the appropriate parameter to
        true (siglab, input4). This will change the available range of input parameters.

        Raises
        ------

            Raises errors if the input parameters are out of range.

        """

        ### Constants ###
        #decimation_list = [1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536]
        gain_list = ["LV","HV"]
        coupling_list = ["DC","AC"]
        units_list = ["RAW","VOLTS"]
        format_list = ["BIN", "ASCII"]

        ### Limits ###
        if input4:   # Set number of channels
            n = 4
        else:
            n = 2
        trig_lvl_lim = 1.0
        gain_lvl = "LV"

        if gain is not None:
            for i in gain:
                if i.upper() == "HV":
                    trig_lvl_lim = 20.0
                    gain_lvl = "HV"

        ### CHECK FOR ERRORS ###
        #try:
        #    assert dec in decimation_list
        #except AssertionError as dec_err:
        #        raise ValueError(f"Decimation needs to be a power of 2 {1, 65536}")

        try:
            assert abs(trig_lvl) <= trig_lvl_lim
        except AssertionError as trig_err:
            raise ValueError(f"Trigger level out of range {-trig_lvl_lim, trig_lvl_lim} V",
                             f"for gain {gain_lvl}") from trig_err

        try:
            assert trig_delay >= 0
        except AssertionError as trig_dly_err:
            raise ValueError("Trigger delay cannot be less that 0") from trig_dly_err

        if units is not None:
            try:
                assert units.upper() in units_list
            except AssertionError as unit_err:
                raise ValueError(f"{units.upper()} is not a defined unit") from unit_err

        if sample_format is not None:
            try:
                assert sample_format.upper() in format_list
            except AssertionError as format_err:
                raise ValueError(f"{sample_format.upper()} is not a defined format") from format_err

        if gain is not None:
            try:
                assert (gain[0].upper() in gain_list) and (gain[1].upper() in gain_list)
            except AssertionError as gain_err:
                raise ValueError(f"{gain[0].upper()} or {gain[1].upper()} is not a defined gain") from gain_err

        if siglab and coupling is not None:
            try:
                assert (coupling[0].upper() in coupling_list) and (coupling[1].upper() in coupling_list)
            except AssertionError as coupling_err:
                raise ValueError(f"{coupling[0].upper()} or {coupling[1].upper()}",
                                 "is not a defined coupling") from coupling_err
            try:
                assert abs(ext_trig_lvl) <= trig_lvl_lim
            except AssertionError as ext_trig_err:
                raise ValueError("External trigger level out of range",
                                 f"{-trig_lvl_lim, trig_lvl_lim} V") from ext_trig_err

        try:
            assert not((siglab is True) and (input4 is True))
        except AssertionError as board_err:
            raise ValueError("Please select only one board option.",
                             "'siglab' and 'input4' cannot be true at the same time.") from board_err


        ### SEND COMMANDS TO RP ###
        self.tx_txt(f"ACQ:DEC {dec}")

        if averaging:
            self.tx_txt("ACQ:AVG ON")
        else:
            self.tx_txt("ACQ:AVG OFF")

        if trig_delay_ns:
            self.tx_txt(f"ACQ:TRIG:DLY:NS {trig_delay}")
        else:
            self.tx_txt(f"ACQ:TRIG:DLY {trig_delay}")

        if units is not None:
            self.tx_txt(f"ACQ:DATA:UNITS {units.upper()}")
        if sample_format is not None:
            self.tx_txt(f"ACQ:DATA:FORMAT {sample_format.upper()}")

        if gain is not None:
            for i in range(n):
                self.tx_txt(f"ACQ:SOUR{i+1}:GAIN {gain[i].upper()}")

        self.tx_txt(f"ACQ:TRIG:LEV {trig_lvl}")

        if siglab and coupling is not None:
            for i in range(n):
                self.tx_txt(f"ACQ:SOUR{i+1}:COUP {coupling[i].upper()}")

            self.tx_txt(f"ACQ:TRIG:EXT:LEV {ext_trig_lvl}")

        #print("ACQ set successfully")

    def get_settings(
        self,
        siglab: bool = False,
        input4: bool = False
    ) -> str:
        """

        Retrieves the settings from Red Pitaya, prints them in console and returns
        them as an array with the following sequence:
        [decimation, avearge, trig_dly, trig_dly_ns, trig_lvl, buf_size, gain_ch1, gain_ch2, coup_ch1, coup_ch2, ext_trig_lvl]
                                                                                           , gain_ch3, gain_ch4
            Decimation   - Current decimation
            Average      - Current averaging status (ON/OFF)
            Trig_dly     - Current trigger delay in samples
            Trig_dly_ns  - Current trigger delay in nanoseconds
            Trig_lvl     - Current triger level in Volts
            Buf_size     - Buffer size
            Gain_ch1-4   - Current gain on channels (CH3 and CH4 STEMlab 125-14 4-Input only)
            Coup_ch1/2   - Current coupling mode for both channels (AC/DC) (SIGNALlab only)
            Ext_trig_lvl - Current external trigger level in Volts (SIGNALlab only)

        Note:   The last three array elements won't exist if siglab = False
                Gain of channels 3 and 4 only if input4 = True

        Parameters
        ----------
            siglab (bool, optional):
                Set to True if operating with SIGNALlab 250-12.
                Defaults to False.
            input4 (bool, optional):
                Set to True if operating with STEMlab 125-14 4-Input.
                Defaults to False.

        """

        try:
            assert not((siglab is True) and (input4 is True))
        except AssertionError as board_err:
            raise ValueError("Please select only one board option. 'siglab' and 'input4' cannot be true at the same time.") from board_err


        settings = []

        if input4:   # Set number of channels
            n = 4
        else:
            n = 2

        settings.append(self.txrx_txt("ACQ:DEC?"))
        settings.append(self.txrx_txt("ACQ:AVG?"))
        settings.append(self.txrx_txt("ACQ:TRIG:DLY?"))
        settings.append(self.txrx_txt("ACQ:TRIG:DLY:NS?"))
        settings.append(self.txrx_txt("ACQ:TRIG:LEV?"))
        settings.append(self.txrx_txt("ACQ:BUF:SIZE?"))

        for i in range(n):
            settings.append(self.txrx_txt(f"ACQ:SOUR{i+1}:GAIN?"))

        if siglab:
            for i in range(2):
                settings.append(self.txrx_txt(f"ACQ:SOUR{i+1}:COUP?"))

            settings.append(self.txrx_txt("ACQ:TRIG:EXT:LEV?"))


        print(f"Decimation: {settings[0]}")
        print(f"Averaging: {settings[1]}")
        print(f"Trigger delay (samples): {settings[2]}")
        print(f"Trigger delay (ns): {settings[3]}")
        print(f"Trigger level (V): {settings[4]}")
        print(f"Buffer size: {settings[5]}")

        if input4:
            print(f"Gain CH1/CH2/CH3/CH4: {settings[6]}, {settings[7]}, {settings[8]}, {settings[9]}")
        else:
            print(f"Gain CH1/CH2: {settings[6]}, {settings[7]}")

        if siglab:
            print(f"Coupling CH1/CH2: {settings[8]}, {settings[9]}")
            print(f"External trigger level (V): {settings[10]}")

        return settings

    def acq_data(
        self,
        chan: int,
        start: int = None,
        end: int = None,
        num_samples: int = None,
        old: bool = False,
        lat: bool = False,
        binary: bool = False,
        convert: bool = False,
        input4: bool = False
    ) -> list:
        """
        Returns the acquired data on a channel from the Red Pitaya, with the following options (for a specific channel):
            - only channel       => returns the whole buffer
            - start and end      => returns the samples between them
            - start and n        => returns 'n' samples from the start position
            - old and n          => returns 'n' oldest samples in the buffer
            - lat and n          => returns 'n' latest samples in the buffer

        Parameters
        ----------
            chan (int) :
                Input channel (either 1 or 2).
                (1-4 for STEMlab 125-14 4-Input)
            start (int, optional):
                Start position of acquired data in the buffer {0,1,...16384}
                Defaults to None.
            end (int, optional):
                End position of acquired data in the buffer {0,1,...16384}
                Defaults to None.
            n (int, optional):
                Number of samples read.
            old (bool, optional):
                Read oldest samples in the buffer.
            lat (bool, optional):
                Read latest samples in the buffer.
            bin (bool, optional):
                Set to True if working with Binary data.
                Defaults to False.
            convert (bool, optional):
                Set to True to convert data to a list of floats (VOLTS) or integers (RAW).
                Otherwise returns a list of str (VOLTS) or int (RAW).
                Defaults to False.
            input4 (bool, optional) :
                Set to True if operating with STEMlab 125-14 4-Input.
                Defaults to False.


        Raises
        ------

            Raises errors if the input parameters do not match one of the options.
        
        """

        low_lim = 0
        up_lim = 16384

        # Check input data for errors
        if input4:
            try:
                assert chan in (1,2,3,4)
            except AssertionError as chanel_err:
                raise ValueError("Channel needs to be either 1, 2, 3 or 4") from chanel_err
        else:
            try:
                assert chan in (1,2)
            except AssertionError as chanel_err:
                raise ValueError("Channel needs to be either 1 or 2") from chanel_err

        try:
            assert not((old is True) and (lat is True))
        except AssertionError as arg_err:
            raise ValueError("Please select only one. 'old' and 'lat' cannot be True at the same time.") from arg_err

        if start is not None:
            try:
                assert 16384 >= start >= 0
            except AssertionError as start_err:
                raise ValueError(f"Start position out of range {low_lim, up_lim}") from start_err

        if end is not None:
            try:
                assert 16384 >= end >= 0
            except AssertionError as end_err:
                raise ValueError(f"End position out of range {low_lim, up_lim}") from end_err

        if num_samples is not None:
            try:
                assert 16384 >= num_samples >= 0
            except AssertionError as sample_err:
                raise ValueError(f"Sample number out of range {low_lim, up_lim}") from sample_err

        # Get data type from Red Pitaya
        units = self.txrx_txt('ACQ:DATA:UNITS?')
        # format = self.txrx_txt("ACQ:DATA:FORMAT?")


        # Determine the output data
        if(start is not None) and (end is not None):
            self.tx_txt(f"ACQ:SOUR{chan}:DATA:STA:END? {start},{end}")

        elif(start is not None) and (num_samples is not None):
            self.tx_txt(f"ACQ:SOUR{chan}:DATA:STA:N? {start},{num_samples}")

        elif old and (num_samples is not None):
            self.tx_txt(f"ACQ:SOUR{chan}:DATA:OLD:N? {num_samples}")

        elif lat and (num_samples is not None):
            self.tx_txt(f"ACQ:SOUR{chan}:DATA:LAT:N? {num_samples}")

        else:
            self.tx_txt(f"ACQ:SOUR{chan}:DATA?")

        # Convert data
        if binary:
            buff_byte = self.rx_arb()

            if convert:
                if units == "VOLTS":
                    buff = [struct.unpack('!f',bytearray(buff_byte[i:i+4]))[0] for i in range(0, len(buff_byte), 4)]
                elif units == "RAW":
                    buff = [struct.unpack('!h',bytearray(buff_byte[i:i+2]))[0] for i in range(0, len(buff_byte), 2)]
            else:
                buff = buff_byte
        else:
            buff_string = self.rx_txt()

            if convert:
                buff_string = buff_string.strip('{}\n\r').replace("  ", "").split(',')
                buff = list(map(float, buff_string))
            else:
                buff = buff_string

        return buff


    def uart_set(
        self,
        speed: int = 9600,
        bits: str = "CS8",
        parity: str = "NONE",
        stop: int = 1,
        timeout: int = 0
    ) -> None:
        """
        Configures the provided settings for UART.

        Args:
            speed (int, optional): Baud rate/speed of UART connection (bits per second). Defaults to 9600.
            bits (str, optional): Character size in bits (CS6, CS7, CS8). Defaults to "CS8".
            parity (str, optional): Parity (NONE, EVEN, ODD, MARK, SPACE). Defaults to "NONE".
            stop (int, optional): Number of stop bits (1 or 2). Defaults to 1.
            timeout (int, optional): Timeout for reading from UART (in 1/10 of seconds) {0,...255}. Defaults to 0.
        """

        # Constants
        speed_list = [1200,2400,4800,9600,19200,38400,57600,115200,230400,576000,921000,1000000,1152000,1500000,2000000,2500000,3000000,3500000,4000000]
        database_list = ["CS6","CS7","CS8"]
        parity_list = ["NONE","EVEN","ODD","MARK","SPACE"]


        # Input Limits Check
        try:
            assert speed in speed_list
        except AssertionError as speed_err:
            raise ValueError(f"{speed} is not a defined speed for UART connection. Please check the speed table.") from speed_err

        try:
            assert bits in database_list
        except AssertionError as bits_err:
            raise ValueError(f"{bits} is not a defined character size.") from bits_err

        try:
            assert parity in parity_list
        except AssertionError as parity_err:
            raise ValueError(f"{parity} is not a defined parity.") from parity_err

        try:
            assert stop in (1,2)
        except AssertionError as stop_err:
            raise ValueError("The number of stop bits can only be 1 or 2") from stop_err

        try:
            assert 0 <= timeout <= 255
        except AssertionError as timeout_err:
            raise ValueError(f"Timeout {timeout} is out of range [0, 255]") from timeout_err

        # Configuring UART

        self.tx_txt("UART:INIT")
        self.tx_txt(f"UART:SPEED {speed}")
        self.tx_txt(f"UART:BITS {bits.upper()}")
        self.tx_txt(f"UART:STOPB STOP{stop}")
        self.tx_txt(f"UART:PARITY {parity.upper()}")
        self.tx_txt(f"UART:TIMEOUT {timeout}")

        self.tx_txt("UART:SETUP")
        print("UART is configured")

    def uart_get_settings(
        self
    ) -> str:
        """
        Retrieves the settings from Red Pitaya, prints them in console and returns
        them as an array with the following sequence:
        [speed, databits, stopbits, parity, timeout]

        """

        # Configuring UART

        settings = []

        settings.append(self.txrx_txt("UART:SPEED?"))
        settings.append(self.txrx_txt("UART:BITS?"))

        stop = self.txrx_txt("UART:STOPB?")
        if stop == "STOP1":
            settings.append("1")
        elif stop == "STOP2":
            settings.append("2")

        settings.append(self.txrx_txt("UART:PARITY?"))
        settings.append(self.txrx_txt("UART:TIMEOUT?"))

        print(f"Baudrate/Speed: {settings[0]}")
        print(f"Databits: {settings[1]}")
        print(f"Stopbits: {settings[2]}")
        print(f"Parity: {settings[3]}")
        print(f"Timeout (0.1 sec): {settings[4]}")

        return settings

    def uart_write_string(
        self,
        string: str,
        word_length: bool = False
    ) -> None:
        """
        Sends a string of characters through UART.

        Args:
            string (str, optional): String that will be sent.
            word_length (bool, optional): Set to True if UART word lenght is set to 7 (ASCII) or
                                    False if UART word length is set to 8 (UTF-8). Defaults to False.
        """

        if word_length:
            # word length 7 / ASCII
            code = "ascii"
        else:
            # word length 8 / UTF-8
            code = "utf-8"


        # transforming and writing to UART
        arr = ',#H'.join(format(x, 'X') for x in bytearray(string, f"{code}"))
        self.tx_txt(f"UART:WRITE{len(string)} #H{arr}")

        print("String sent")

    def uart_read_string(
        self,
        length: int
    ) -> str:
        """
        Reads a string of data from UART and decodes it from ASCII to string.

        Args:
            length (int): Length of data to read from UART.

        Returns:
            str: Read data in string format.
        """

        # Check for errors
        try:
            assert length > 0
        except AssertionError as length_err:
            raise ValueError("Length must be greater than 0.") from length_err

        self.tx_txt(f"UART:READ{length}")
        res = self.rx_txt()
        res = res.strip('{}\n\r').replace("  ", "").split(',')
        string = "".join(chr(int(x)) for x in res)  # int(x).decode("utf8")

        return string


    def spi_set(
        self,
        spi_mode: str = None,
        cs_mode: str = None,
        speed: int = None,
        word_len: int = None
    ) -> None:
        """
        Configures the provided settings for SPI.

        Args:
            spi_mode (str, optional): Sets the mode for SPI; - LISL (Low Idle level, Sample Leading edge)
                                                             - LIST (Low Idle level, Sample Trailing edge)
                                                             - HISL (High Idle level, Sample Leading edge)
                                                             - HIST (High Idle level, Sample Trailing edge)
                                                        Defaults to LISL.
            cs_mode (str, optional): Sets the mode for CS: - NORMAL (After message transmission, CS => HIGH)
                                                           - HIGH (After message transmission, CS => LOW)
                                                        Defaults to NORMAL.
            speed (int, optional): Sets the speed of the SPI connection. Defaults to 5e7.
            word_len (int, optional): Character size in bits (CS6, CS7, CS8). Defaults to "CS8".
        """

        # Constants
        speed_max_limit = 100e6
        speed_min_limit = 1
        cs_mode_list = ["NORMAL","HIGH"]
        order_list = ["MSB","LSB"]
        spi_mode_list = ["LISL","LIST","HISL","HIST"]
        bits_min_limit = 7


        # Input Limits Check

        try:
            assert spi_mode.upper() in spi_mode_list
        except AssertionError as spi_mode_err:
            raise ValueError(f"{spi_mode} is not a defined SPI mode.") from spi_mode_err

        try:
            assert cs_mode.upper() in cs_mode_list
        except AssertionError as cs_err:
            raise ValueError(f"{cs_mode} is not a defined CS mode.") from cs_err

        try:
            assert speed_min_limit <= speed <= speed_max_limit
        except AssertionError as speed_err:
            raise ValueError(f"{speed} is out of range [{speed_min_limit},{speed_max_limit}].") from speed_err

        try:
            assert word_len >= bits_min_limit
        except AssertionError as bits_err:
            raise ValueError(f"Word length must be greater than {bits_min_limit}. Current word length: {word_len}") from bits_err


        # Configuring SPI

        self.tx_txt(f"SPI:SET:MODE {spi_mode.upper()}")
        self.tx_txt(f"SPI:SET:CSMODE {cs_mode.upper()}")
        self.tx_txt(f"SPI:SET:SPEED {speed}")
        self.tx_txt(f"SPI:SET:WORD {word_len}")

        self.tx_txt("SPI:SET:SET")
        print("SPI is configured")

    def spi_get_settings(
        self
    ) -> str:
        """
        Retrieves the SPI settings from Red Pitaya, prints them in console and returns
        them as an array with the following sequence:
        [mode, csmode, speed, word_len, msg_size]

        """

        # Configuring SPI

        self.tx_txt("SPI:SET:GET")
        settings = []

        settings.append(self.txrx_txt("SPI:SET:MODE?"))
        settings.append(self.txrx_txt("SPI:SET:CSMODE?"))
        settings.append(self.txrx_txt("SPI:SET:SPEED?"))
        settings.append(self.txrx_txt("SPI:SET:WORD?"))
        settings.append(self.txrx_txt("SPI:MSG:SIZE?"))

        print(f"SPI mode: {settings[0]}")
        print(f"CS mode: {settings[1]}")
        print(f"Speed: {settings[2]}")
        print(f"Word length: {settings[3]}")
        print(f"Message queue length: {settings[4]}")

        return settings


# IEEE Mandated Commands

    def cls(self):
        """Clear Status Command"""
        return self.tx_txt('*CLS')

    def ese(self, value: int):
        """Standard Event Status Enable Command"""
        return self.tx_txt(f'*ESE {value}')

    def ese_q(self):
        """Standard Event Status Enable Query"""
        return self.txrx_txt('*ESE?')

    def esr_q(self):
        """Standard Event Status Register Query"""
        return self.txrx_txt('*ESR?')

    def idn_q(self):
        """Identification Query"""
        return self.txrx_txt('*IDN?')

    def opc(self):
        """Operation Complete Command"""
        return self.tx_txt('*OPC')

    def opc_q(self):
        """Operation Complete Query"""
        return self.txrx_txt('*OPC?')

    def rst(self):
        """Reset Command"""
        return self.tx_txt('*RST')

    def sre(self):
        """Service Request Enable Command"""
        return self.tx_txt('*SRE')

    def sre_q(self):
        """Service Request Enable Query"""
        return self.txrx_txt('*SRE?')

    def stb_q(self):
        """Read Status Byte Query"""
        return self.txrx_txt('*STB?')

# :SYSTem

    def err_c(self):
        """Error count."""
        return self.txrx_txt('SYST:ERR:COUN?')

    def err_n(self):
        """Error next."""
        return self.txrx_txt('SYST:ERR:NEXT?')
