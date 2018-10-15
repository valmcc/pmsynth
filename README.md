# pmsynth (alpha)
A waveguide synth for the stm32f4, that runs on the STM32F4-DISC1 development board.
Synth code is in the /pmsynth directory. 

# Implemented models

* 1D waveguide strings and tubes
* Banded waveguide model for xylophones and marimbas
* Flute model
* 2D Mesh model - this isn't viable yet due to computational limitiations

# Notes

* 44100 samples/sec at 16 bits/sample
* Takes MIDI input on the A3 port (needs to go via an optocoupler circuit first)
* 12 note polyphony at the moment
* All synth code is in floating points.

# How do I build this?

* [GNU Make](https://www.gnu.org/software/make/)
* [GNU Embedded Toolchain (ARM)](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads)

Then you need to run 
` make clean && make all`
To program run
`make program`

Backend (driver) code and some underlying audio processing is based off Jason Harris' work [here](https://github.com/deadsy/googoomuck) instead of HAL or CMSIS.

