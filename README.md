# Oscilloscope
Simple two channel oscilloscope, function generator and bode plotter for STM32 NUCLEO-F446RE board.

![Picture of oscilloscope measuring effect of LED on DAC channel 2, when internal DAC buffer is disabled](https://raw.githubusercontent.com/19greg96/Oscilloscope/master/photo.jpg)
Picture of oscilloscope measuring effect of LED on DAC channel 2, when internal DAC buffer is disabled.


## Issues with the hardware design:
 - Input voltage range is bad (5Vpp). This can be extended by switchable voltage divider on input.
 - No separate analog / digital ground plane.
 - Input trace has no extra clearance from ground planes.
 - GPIO pin allocation is not optimised for header side, leading to many cross board signal connections.
 - No control of anti-alias filter cut-off frequency.
 - No hardware gain control (op-amp reference should be at Vcc/2).
 - In DC coupling mode, there is a Vcc/2 offset caused by the bias for AC coupling.
 - There is an offset voltage on channel A for some reason (this could be fixed in software, but hardware explanation would be better).
 - AC coupling cutoff frequency could be lower.
 - AC coupling cutoff frequency is changed by external resistance, because there is no buffer directly on the input. (buffer needs a negative voltage rail for this)
 - DAC output single resistor is not enough for power rating. Minimum two parallel resistors are needed for 3.3V max output.
 - Max DAC output voltage is little less than Vref.
 - No support of negative DAC output.
 - Rotary encoder is too close to buttons and GLCD.
 - DC and IO jacks do not hang far enough off of PCB edge to enable boxing of device.
 - GLCD is upside-down making contrast bad at normal viewing angles.
 - No support fot 4W measurement.
 
