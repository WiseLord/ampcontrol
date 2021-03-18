# Ampcontrol (ATmega16 version)

Ampcontrol project is a control module for your DIY audio amplifier.
It supports various I²C/SPI-controlled audioprocessors and FM tuners.

## Main features:

- Amplifier control (mute/standby external signals)
- Digital audio control (feature set depends on used audio processor)
- 32-band spectrum analyzer (0..5kHz)
- English, Russian, Belarusian, Ukrainian and possible other localizations
- FM radio chips support / up to 50 FM stations can be saved in EEPROM
- 10 favourite FM stations support (mapped to 0..9 buttons of remote)
- RC5 remotes support

## Supported displays:

- [KS0108 type A](files/sch/ampcontrol_gd_a.png) - Monochrome 128x64 graphic display with direct CS1/CS2 polarity
- [KS0108 type B](files/sch/ampcontrol_gd_b.png) - Monochrome 128x64 graphic display with inverted CS1/CS2 polarity
- [KS0066 16x2](files/sch/ampcontrol_lcd.png) - Monochrome alphanumeric display

## Supported audio processors:

- TDA7439 - 4 stereo inputs, bass, middle, treble
- TDA7312 - 4 stereo inputs, bass, treble
- TDA7313/PT2313 - 3 stereo inputs, bass, treble, fade, loudness
- TDA7314 - Stereo input, bass, treble, fade, loudness
- TDA7315 - Stereo input, bass, treble
- TDA7318 - 4 stereo inputs, bass, treble, fade
- PT2314 - 4 stereo inputs, bass, treble, loudness
- TDA7448 - 6-ch input, fade, center, subwoofer
- PT2323/PT2322 - 4 stereo inputs, 5.1 input, bass, treble, fade, center, subwoofer, surround, 3d
- TEA6300 - 3 stereo inputs, bass, treble, fade
- TEA6330 - Stereo input, bass, treble, fade
- PGA2310 - High-end stereo volume control (ATmega32 only)
- TUNER_VOLUME - Use built-in tuner (RDA580x/Si4703) volume control as audioprocessor
- R2S15904SP - 4 stereo inputs, bass, treble (found in Microlab Solo-7C) (Atmega32 only)

## Supported FM tuners

- TEA5767 - I²C FM tuner
- RDA5807 - I²C FM tuner with RDS support
- TUX032  - I²C FM tuner found in some Sony car radio
- LM7001  - SPI-controlled frequency synthesizer
- RDA5802 - I²C FM tuner
- SI470X  - I²C FM tuner with RDS support
- LC72131 - SPI-controlled frequency synthesizer

## Building the code

GNU AVR Embedded Toolchain is required to build the project.

The project build system is based on [GNU Make](https://www.gnu.org/software/make/).
Linux users must have it already installed or they can easily do it.
For Windows users there is a port exists.
In both cases, both toolchain and make binaries should be added to the system PATH.

The process of building is very simple and doesn't depend on any other software installed.

### Default build (from src/ directory of the project):

`make`

### Build for the specific hardware

`make DISPLAY=KS0066_16X2_PCF8574 AUDIOPROC=PT232X TUNER=TUX032`

The list of supported display controllers and other hardware can be found in [Makefile](src/Makefile) or [build_all](build_all.sh) script.

## Schematics and wiring

The schematic and PCB files for the device itself can be found in [files](files) directory.

## Ampcontrol EEPROM editor

While project uses various parameters from EEPROM memory (for example, audioprocessor and tuner selection),
special desktop application to edit eeprom_xx.bin was designed. It allows to load, modify and save eeprom binary file in easy way.

![Ampcontrol EEPROM editor](doc/editor.png)

Ampcontrol editor is written with Qt5 and the source code is availiable [here](editor)

## Useful links

- [Releases](https://github.com/WiseLord/ampcontrol/releases) of Ampcontrol firmwares and related software.
- [Youtube playlist](https://www.youtube.com/watch?v=yy9XiIlWfCc&list=PLfv57leyFFd3KTFyBgsR7_01ZDTgWIuI9) (mostly in Russian) about Ampcontrol
- [Article about the project](http://radiokot.ru/circuit/audio/other/39) (in Russian)
- [Forum thread](http://radiokot.ru/forum/viewtopic.php?t=98758) (in Russian)
