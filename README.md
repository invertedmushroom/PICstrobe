PICstrobe – Stroboscopic Tachometer with PIC16F877A

PICstrobe is a stroboscopic tachometer built around the PIC16F877A microcontroller, featuring:  
A 2-line character LCD  
A high-power 3.8 W LED (overdriven with limited on-time)  
Rotary encoder + buttons for menu navigation  
Interrupt-based LED modulation for accurate timing  

✨ Features

Manual LED frequency setting – set the LED blink frequency directly  
Rotor slip calculator – enter stator frequency, rotor RPM, and pole pairs to calculate slip  
Menu navigation – intuitive rotary encoder + buttons  
LCD interface – clear display of values and feedback  

🔦 LED Drive and Timing

ON-time is fixed to ~1.4 ms  
OFF-time is variable and depends on user-selected frequency  

⏱ Timer Configuration

Timer0 is configured to overflow every 200 instruction cycles  
At a 4 MHz crystal, 1 instruction = 1 µs → overflow every 200 µs  
This is used as a time base for all timing in the system  

🧮 Time Calculation

To generate a blinking pattern:  
A counter (st_i) is incremented every 200 µs (in the Timer0 ISR)  
When st_i >= r_i, the LED is turned on for a fixed 1.4 ms (using a short nop loop)  
Afterward, the LED is turned off, and st_i is reset  

OFF time = r_i × 200 µs  
ON time ≈ 1.4 ms (fixed)  
Total period ≈ OFF time + ON time  
Frequency ≈ 1 / (OFF time + ON time)  
The value r_i is computed from the desired frequency or RPM and adjusted by subtracting ~7 interrupts (1.4 ms / 200 µs), which compensates for the LED ON duration and interrupt overhead (measured with oscilloscope).  

🎛 Pulse-Density Modulation

This LED drive method is functionally equivalent to PDM (Pulse-Density Modulation):  
The pulse width (ON-time) is fixed  
The density of pulses (how often they occur) varies with the input RPM or frequency  

💻 Development Info

Target platform: PICDEM 2 PLUS  
MCU: PIC16F877A  
Compiler: Hi-Tech C Compiler (Microchip)  
