5 channel CV|midi converter
===


![My image](https://c2.staticflickr.com/2/1780/43105246015_8cc601f7be_h.jpg)


### mmh?

- 5 x dual S/H units to drive midi equipment ( ... i got a nord drum 2 for cheap, hence was under the impression i needed something like this).
- with a little more time/effort, this could be made more compact and so on (i was briefly considering using something cheaper/more suitable (STM32F373R8), but ended up with t3.2 again)
- internally, the module re-utilizes the o_C - version of the MI "braids" quantizer; it tracks V/oct fairly ok.
- also features configurable trigger-to-SH "delay" (latency) settings (typically helps when re-quantizing control voltages)
- CV inputs default to note/velocity; alternatively velocity can be mapped to any CC message ("learn").
- there's a micro USB connector on the backside of the module, so in theory there's the option to also have usb-midi (not implemented, and somewhat inconvenient)

### hardware:

- trigger/gate in, V/oct, velocity (CV + manual offset) per channel; midi TRS in/out
- 15kHz update rate
- teensy 3.2 based
- 10HP, depth ~ 35mm
