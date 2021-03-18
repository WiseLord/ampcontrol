# Ampcontrol (ATmega8-lcd version)

Ampcontrol project is a control module for your DIY audio amplifier.
This branch supports only TDA7313 audioprocessor.

## Main features:

- Amplifier control (mute/standby external signals)
- Digital audio control (feature set depends on used audio processor)
- 32-band spectrum analyzer (0..5kHz)
- English and Russian localizations
- RC5 remotes support with leargning mode

## Supported display:

- [KS0066 16x2](files/sch-m8.png) - Monochrome alphanumeric display 16x2

## Supported audio processor:

- TDA7313/PT2313 - 3 stereo inputs, bass, treble, fade, loudness

## Building the code

GNU AVR Embedded Toolchain is required to build the project.

The project build system is based on [GNU Make](https://www.gnu.org/software/make/).
Linux users must have it already installed or they can easily do it.
For Windows users there is a port exists.
In both cases, both toolchain and make binaries should be added to the system PATH.

The process of building is very simple and doesn't depend on any other software installed.

### Default build (from src/ directory of the project):

`make`

## Schematics and wiring

The schematic and PCB files for the device itself can be found in [files](files) directory.

## Useful links

- [Releases](https://github.com/WiseLord/ampcontrol/releases) of Ampcontrol firmwares and related software.
- [Youtube playlist](https://www.youtube.com/watch?v=yy9XiIlWfCc&list=PLfv57leyFFd3KTFyBgsR7_01ZDTgWIuI9) (mostly in Russian) about Ampcontrol
- [Article about the project](http://radiokot.ru/circuit/audio/other/39) (in Russian)
- [Forum thread](http://radiokot.ru/forum/viewtopic.php?t=98758) (in Russian)
