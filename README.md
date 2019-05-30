# Oscilloscope
Simple two channel oscilloscope, function generator and bode plotter for STM32 NUCLEO-F446RE board.

![Picture of oscilloscope measuring effect of LED on DAC channel 2, when internal DAC buffer is disabled](https://raw.githubusercontent.com/19greg96/Oscilloscope/master/photo.jpg)


## Issues with the hardware design:
 - No hardware gain control (op-amp reference should be at Vcc/2).
 - Input voltage range is bad (5Vpp). This can be extended by switchable voltage divider on input.
 - No control of anti-alias filter cut-off frequency.
 - Anti-alias filter cut-off frequency is too low.
 - In DC coupling mode, there is a Vcc/2 offset caused by the bias for AC coupling.
 - AC coupling cutoff frequency could be lower.
 - AC coupling cutoff frequency is changed by external resistance, because there is no buffer directly on the input.
 - There is an offset voltage on channel A for some reason (this could be fixed in software, but hardware explanation would be better).
 - DAC output amp Rout = 150Ohm when new op-amp (TLV3544IDR) supports 50Ohm or even less.
 - DAC output can't be connected to ADC input before current limiting resistor, for bode measurement (best case would be hardware support).
 - Max DAC output voltage is little less than Vref.
 - No support of negative DAC output.
 - GLCD backlight and relay driver FET gates are not pulled down.
 - Components are spaced sparsely thus wasting PCB space.
 - Top of PCB is virtually wasted space.
 - Input trace has no extra clearance from ground planes.
 - Contrast setting pot footprint is incorrect.
 - IO 3.5mm jack footprint is incorrect.
 - Rotary encoder is too close to buttons and GLCD.
 - DC and IO jacks do not hang far enough off of PCB edge to enable boxing of device.
 - GLCD is upside-down making contrast bad at normal viewing angles.
 - No support fot 4W measurement.
 - No support for RLC component testing.
 
