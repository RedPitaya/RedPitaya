from ctypes import *
import time

rp_api = CDLL('/opt/redpitaya/lib/librp.so')

CH_1 = 0
CH_2 = 1

TRIG_SRC_DISABLED = 0 # Trigger is disabled
TRIG_SRC_NOW      = 1 # Trigger triggered now (immediately)
TRIG_SRC_CHA_PE   = 2 # Trigger set to Channel A threshold positive edge
TRIG_SRC_CHA_NE   = 3 # Trigger set to Channel A threshold negative edge
TRIG_SRC_CHB_PE   = 4 # Trigger set to Channel B threshold positive edge
TRIG_SRC_CHB_NE   = 5 # Trigger set to Channel B threshold negative edge
TRIG_SRC_EXT_PE   = 6 # Trigger set to external trigger positive edge (DIO0_P pin)
TRIG_SRC_EXT_NE   = 7 # Trigger set to external trigger negative edge (DIO0_P pin)
TRIG_SRC_AWG_PE   = 8 # Trigger set to arbitrary wave generator application positive edge
TRIG_SRC_AWG_NE   = 9 # Trigger set to arbitrary wave generator application negative edge

WAVEFORM_SINE      = 0 # Wave form sine
WAVEFORM_SQUARE    = 1 # Wave form square
WAVEFORM_TRIANGLE  = 2 # Wave form triangle
WAVEFORM_RAMP_UP   = 3 # Wave form sawtooth (/|)
WAVEFORM_RAMP_DOWN = 4 # Wave form reversed sawtooth (|\)
WAVEFORM_DC        = 5 # Wave form dc
WAVEFORM_PWM       = 6 # Wave form pwm
WAVEFORM_ARBITRARY = 7 # Use defined wave form

TRIG_STATE_TRIGGERED = 0 # Trigger is triggered/disabled
TRIG_STATE_WAITING   = 1 # Trigger is set up and waiting (to be triggered)


class base:
	def Init():
		return rp_api.rp_Init()

	def Release():
		return rp_api.rp_Release()

	def GenReset():
		return rp_api.rp_GenReset()

	def GenFreq(channel, freq):
		return rp_api.rp_GenFreq(channel, c_float(freq))

	def GenAmp(channel, freq):
		return rp_api.rp_GenAmp(channel, c_float(freq))
	
	def GenWaveform(channel, form):
		return rp_api.rp_GenWaveform(channel, form)

	def GenOutEnable(channel):
		return rp_api.rp_GenOutEnable(channel)

	def AcqReset():
		return rp_api.rp_AcqReset()

	def AcqSetDecimation(dec_factor):
		return rp_api.rp_AcqSetDecimation(dec_factor)

	def AcqSetTriggerLevel(channel, level):
		return rp_api.rp_AcqSetTriggerLevel(channel, c_float(level))

	def AcqSetTriggerDelay(delay):
		return rp_api.rp_AcqSetTriggerDelay(delay)

	def AcqStart():
		return rp_api.rp_AcqStart()

	def AcqSetTriggerSrc(source):
		return rp_api.rp_AcqSetTriggerSrc(source)

	def AcqGetTriggerState(state):
		return rp_api.rp_AcqGetTriggerState(byref(state))

	def AcqGetOldestDataV(channel, buff_size, buff):
		return rp_api.rp_AcqGetOldestDataV(0, byref(buff_size), byref(buff));

	def DpinSetState(pin, state):
		return rp_api.rp_DpinSetState(pin, state)

	def AIpinGetValue(pin, value):
		return rp_api.rp_AIpinGetValue(pin, byref(value))

	def AOpinSetValue(pin, value):
		return rp_api.rp_AOpinSetValue(pin, c_float(value))

class misc:
	def CreateFloatBuffer(size):
		return (c_float*size)()


