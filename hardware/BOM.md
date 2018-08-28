# BOM for cv | midi

### resistors 0603:

| value	| # | note |
| ---	| ---: | --- |
| 47R	|	2x	|	|
| 100R	|	10x |	|
| 220R	|	2x |	|
| 1k	|	1x |	|
| 10k	|	2x | + 6x (if/when using ultrabright LEDs (recommended))	|
| 33k	|	15x |	|
| 100k	|	20x |	|
| 200k	|	5x |	|

### capacitors 0603:

| value	| # | note |
| ---	| ---: | --- |
| 220p-330p	|	10x	| 50V (C0G/NP0) |
| 100n	|	5x	| 16V (or better) |
| 1uF	|	1x	| 25V (or better) |
| 10uF	|	4x	| 25V (or better) (0805 is ok, too) |

### capacitors (electrolytic / SMD):

| value	| # | note |
| ---	| ---: | --- |
| 22uF-47uF	|	2x | 25V (or better) e.g. UUD1H220MCL1GS (6.3mm) |

### ICs/semis/etc:

| what | # | package | note |
| --- | ---: | --- | --- |
| 6N137 (optocoupler) | 1x | SMD | e.g. mouser # 512-6N137SVM |
| MCP6002 | 1x | SOIC8 | mouser # 579-MCP6002T-I/SN |
| MCP6004 | 2x | SOIC14 | mouser # 579-MCP6004T-I/SL |
| MMBT3904 | 5x | SOT-23 | mouser # 512-MMBT3904 |
| ADP150 | 1x | TSOT | 3v3 regulator; mouser # 584-ADP150AUJZ-3.3R7 |
| LM1117-5 | 1x | SOT-223 | 5v0 regulator; mouser # 511-LD1117S50 |
| LM4040-10v | 1x | SOT-23 | e.g. mouser # 926-LM4040DIM3X10NPB |
| SM5817PL-TP | 2x | SOD-123FL | mouser # 833-SM5817PL-TP |
| 1N4148 | 1x | SOD-323 | mouser # 833-1N4148WX-TP |
| Teensy 3.2 dev board | 1x | - | (Teensy LC might/should work, too .. I haven't tried) |

### misc:

| what | # | note |
| --- | ---: | --- |
| C&K 1P2T SMT SUBMIN | 2x | mouser # 611-AYZ0202AGRLC |
| switch OFF - (ON) (E-switch )| 1x | mouser # 612-TL1105SP-250 |
| + cap | 1x | mouser # 612-1R-BK (black) |
| PJ398SM (mono 3.5mm) | 15x | thonkiconn / kobiconn |
| PJ366ST (stereo 3.5mm) | 2x | thonkiconn "green stereo" (or 2.5mm, if you can find one with the same footprint / if you want to comply with the TRS midi spec) |
| potentiometers ("tall trimmers") | 5x | 10k / 9mm |
| LEDs 3mm warm white | 5x | or whatever you prefer (make sure to use suitable resistors) |
| LEDs 3mm diffuse blue | 1x | or whatever you prefer (make sure to use suitable resistors) |
| 2.54mm 2x5 header | 1x | e.g. mouser # 649-67996-410HLF |
| 2.54 1x4 header | 4x | (use breakable pin headers) |
| 2.54 1x10 header | 1x | (use breakable pin headers) |
| 2.54 1x16 header | 1x | (use breakable pin headers) |
| 2.54 1x4 socket | 4x | - |
| 2.54 1x10 socket | 1x | - |
| 2.54 1x16 socket | 1x | - |
| 2.54 headers/sockets for Teensy (1x14) | 2x each | (the pcbs holes are fairly tight, so preferable use "machined" pins) |
