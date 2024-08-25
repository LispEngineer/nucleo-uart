# Nucleo MIDI Test

* Author: [Douglas P. Fields, Jr.](mailto:symbolics@lisp.engineer)

Specs:
* Board: ST Nucleo-F767ZI
* MIDI: [ubld.it MIDI-Mini Breakout Board](https://ubld.it/midi-mini)

Software:
* STM32CubeIDE
* STM32 HAL libraries

Documentation:
* [STM32F7 HAL, UM1905](https://www.st.com/resource/en/user_manual/um1905-description-of-stm32f7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
* [STM32 Cube IDE User Guide, UM2609](https://www.st.com/resource/en/user_manual/um2609-stm32cubeide-user-guide-stmicroelectronics.pdf)
* [STM32 Cube MX User GUide, UM1718](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf)
* [STM32F767xx Datasheet](https://www.st.com/resource/en/datasheet/stm32f765bi.pdf)
* [RM0410 STM32F76 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
* [UM1974 Nucleo-144 User Manual](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
* [Arm Cortex-M7 Technical Reference Manual](https://developer.arm.com/documentation/ddi0489/latest/)
* [Arm v7-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0403/latest/)
* [MIDI 1.0](https://midi.org/midi-1-0)
* [MIDI 1.0 Core Specifications](https://midi.org/midi-1-0-core-specifications)
* [MIDI 1.0 MPE](https://midi.org/midi-1-0-detailed-specification)

Goals:
* Get UART transfer working for ST-LINK console
* Get UART transmit working for MIDI
  * Send a note on and off regularly
* Get UART receive working for MIDI
* Get DMA UART working
* Build something simple:
  * MIDI receive
  * Text translation of MIDI over Serial
  * MIDI retransmit (e.g., a THRU with a bit of latency)
  * Measure latency
  * Minimize latency