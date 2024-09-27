import rp
import time 
import numpy as np
import matplotlib.pyplot as plt

def get_user_input(prompt, default):
    user_input = input(f"{prompt} (default: {default}):")
    return float(user_input)if user_input else default 

# Get user-defined parameters
frequency = get_user_input("Enter the frequency (Hz)", 1)
duty_cycle = get_user_input("Enter the duty cycle (0 to 1)", 0.1)
duration = get_user_input("Enter the duration for each cycle (seconds)", 10)
sampling_rate = 1000 

# Calculations for number of samples + time and input data
num_samples = int(sampling_rate * duration) 
time_data = np.linspace(0, duration, num_samples)
input_data = np.zeros(num_samples)

# Calculations for period and high/low times
period = 1.0 / frequency
high_time = period * duty_cycle
low_time = period * (1 - duty_cycle)

def output_square_wave(frequency, duty_cycle, duration):
    period = 1.0 / frequency
    high_time = period * duty_cycle
    low_time = period * (1 - duty_cycle)
    
    start_time = time.time()
    sample_index = 0 
    sample_interval = 1.0 / sampling_rate
    
    while (time.time() - start_time) < duration:
        rp.rp_DpinSetState(rp.RP_DIO1_P, rp.RP_HIGH)
        for a in range(int(high_time * sampling_rate)):
            input_data[sample_index] = 1
            sample_index += 1
            time.sleep(sample_interval)

        rp.rp_DpinSetState(rp.RP_DIO1_P, rp.RP_LOW)
        for a in range(int(low_time * sampling_rate)):
            input_data[sample_index] = 0
            sample_index += 1
            time.sleep(sample_interval)
            
# Output square wave
try:
    print(f"\nOutputting a {duty_cycle*100}% duty cycle PWM at {frequency} Hz on pin {output_pin}.")
    print("Press Ctrl-C to quit.")

    while True:
        output_square_wave(frequency, duty_cycle, duration) # Calls function to generate square wave 

        user_continue = input("\nContinue generating square waves? (y/n):").strip().lower()
        if user_continue == 'n':
            break 
        frequency = get_user_input("Enter the frequency (Hz)", frequency)
        duty_cycle = get_user_input("Enter the duty cycle (0 to 1)", duty_cycle)
        duration = get_user_input("Enter the duration for each cycle (seconds)", duration)
        
except KeyboardInterrupt:
    print('\n\nKeyboard interrupt. Exiting the program') # Ctrl + C for interrupt

finally:
    rp.rp_DpinSetState(rp.RP_DIO1_P, rp.RP_LOW)  # Set pin to low
    rp.rp_Release() # Release used resources
    
# Plots output 
plt.figure(figsize=(10, 4))
plt.step(time_data[:len(input_data)], input_data, where='post', label="GPIO Output Signal", linewidth=2)
plt.title("Square Wave Output from Red Pitaya")
plt.xlabel("Time (s)")
plt.ylabel("State (High/Low)")
plt.yticks([0, 1], ['Low', 'High'])  
plt.grid(True)
plt.legend()
plt.ylim(-0.1, 1.1)  
plt.xlim(0, duration)  
plt.show()      
