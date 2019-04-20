# PICstrobe
Strobelight Tachometer Using PIC16F877A, 2 line LCD and 3,8W LED (LED current limited with resistor at ~150%
of nominal value, since ON time is 7 clock cycles)
Strobelight tachometer features a menu with selection between:
-setting LED flashing frequency
-interactive menu for calculating electric motor's rotor slip (user inputs it's fixed stator frequency)
-measurment of frequency
-outside trigger
Menu selection and frequency adjustment is done with rotary decoder and few buttons.
Code is writen for PICDEM 2 PLUS and compiled with "Hi-Tech C compiler" from Microchip. Frequency measurment and outside trigger not implemented in this version.
