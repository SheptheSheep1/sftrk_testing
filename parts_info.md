# sftrk Parts Knowledge Base
## Screen
- ![ssd1306](https://www.datasheethub.com/wp-content/uploads/2022/08/SSD1306-128x64-I2C-OLED-Display-720x720.png)
- [Store Listing(Budget)](https://www.amazon.com/AITRIP-Display-Self-Luminous-Compatible-Raspberry/dp/B0CT5CJWFK?crid=25WJD2EDOKI47&s=electronics&sprefix=0.96%2Binch%2Boled%2Bssd1306%2Celectronics%2C111&sr=1-11&th=1)
- I2C Display using SSD1306 IC, 128x64, no embedded fonts
- Info listing at [52pi](https://wiki.52pi.com/index.php?title=S-0005)
- ^ see above link for using fonts and integrating images/icons
- font arr generator [link](https://oleddisplay.squix.ch/)
- Address set at `0x3C` by default
- Potential libraries: `u8g2`, `adafruit ssd1306/GFX`, could also implement manually by writing to address over I2C

## LoRa Module (HT-RA62 902~928MHz) ([link](https://heltec.org/project/ht-ra62/))
- ![HT-RA62 Back](https://heltec.org/wp-content/uploads/2023/10/RA62-1-600x600-1.png)
- ![HT-RA62 Top](https://heltec.org/wp-content/uploads/2023/10/RA62-2.png)
- [schematic](https://resource.heltec.cn/download/HT-RA62/HT-RA62_Reference_design.pdf)
- [pinouts](https://resource.heltec.cn/download/HT-RA62/HT-RA62.png)
- [all data](https://resource.heltec.cn/download/HT-RA62)
- uses [SX1262](https://cdn.sparkfun.com/assets/6/b/5/1/4/SX1262_datasheet.pdf) transceiver
- currently everything is set up for ~915MHz
- Potential libraries/abstractions: `radiolib`, `radiohead`

## Main MCU (nrf52840) (board: ProMicro)
- ![nice!nanov2 pinout](https://raw.githubusercontent.com/pdcook/nRFMicro-Arduino-Core/main/supermini_nicenano.png)
- names: Pro Micro, Promicro, Nice!Nano, SuperMini nRF52840
- the ProMicro is designed as a [nice!nanov2](https://nicekeyboards.com/nice-nano/) board clone, usually used for keyboards (split keyboards and such)
- This project (called [nrfMicro](https://github.com/joric/nrfmicro/wiki)) is a drop-in replacement for this board but it documents people buying ProMicro boards and the issues people have with it, as well of the different iterations of it
- Git repository with bindings for platformio [link](https://github.com/ICantMakeThings/Nicenano-NRF52-Supermini-PlatformIO-Support/tree/main)
- this repository just provides a lot of simple ease-of-use bindings like pins and things like that
- [Zephyr OS documentation for this board](https://docs.zephyrproject.org/latest/boards/others/promicro_nrf52840/doc/index.html)
- we will consider switching from arduino to zephyr OS in the future

## GPS Modules (ATGM336H) [Datasheet](https://datasheet.lcsc.com/lcsc/1810261521_ZHONGKEWEI-ATGM336H-5N31_C90770.pdf)
- ![ATGM336H 5N Top](https://ae01.alicdn.com/kf/HTB1dj3uagFY.1VjSZFqq6ydbXXaC.jpg)
- cheap board, operated as serial device producing [NMEA](https://aprs.gids.nl/nmea/) sentences at 9600 baud
- Potential libraries: `TinyGPSPlus`, `NeoGPS`
