5 channel CV|midi converter
===


![My image](https://c2.staticflickr.com/2/1780/43105246015_8cc601f7be_h.jpg)


### mmh?

- 5 x dual S/H units to drive midi equipment ( ... i got a nord drum 2 for cheap, so i whipped this up).

- with a little more time/effort, this could be made more compact and so on (i was briefly considering using something cheaper/more suitable, i suppose (STM32F373R8), but ended up with t3.2 again)

- internally, the module re-utilizes the o_C - version of the MI "braids" quantizer; it tracks V/oct fairly ok.

- also features configurable trigger-to-SH "delay" (latency) settings (typically helps when re-quantizing control voltages)

### hardware:

eurorack / teensy 3.2 

- trigger/gate in, V/oct, velocity (CV + manual offset) per channel; midi TRS in/out

- 15kHz update rate

- 10HP, depth ~ 35mm
