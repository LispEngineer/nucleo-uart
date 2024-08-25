# Nucleo MIDI Test

* Author: [Douglas P. Fields, Jr.](mailto:symbolics@lisp.engineer)
* Copyright 2024, Douglas P. Fields Jr.
* License: [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)

Hardware:
* Board: [ST Nucleo-F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html)
* MIDI: [ubld.it MIDI-Mini Breakout Board](https://ubld.it/midi-mini)
* [CME U2MIDI PRO](https://www.cme-pro.com/u2midi-pro-usb-to-midi-cable/)
  USB MIDI interface

Software:
* STM32CubeIDE
* STM32 HAL libraries
* [UxMIDI tools](https://www.cme-pro.com/start-guide-for-uxmidi-tools-software-by-cme/)

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
* DONE - Get UART transfer working for ST-LINK console
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

  
# MIDI Notes

References:
* [MIDI tutorial for programmers](https://www.cs.cmu.edu/~music/cmsip/readings/MIDI%20tutorial%20for%20programmers.html)

MIDI electrical protocol:
* 31.25 Kbaud ±1%
* asynchronous
* start bit, 8 data bits, stop bits
* start bit is a logical 0 (current on)
* stop bit is a logical 1 (current off)
* See: Midi 1.0 Detailed Specification 4.2.1 p1

MIDI data protocol briefly:
  * STATUS byte (bit 7 is set) followed by
  * DATA bytes (bit 7 is reset)
  * STATUS bytes can be omitted if they are the same
    * Called "running status"

Note on & off:
* ON Status byte : 1001 CCCC
* Data byte 1 : 0PPP PPPP
* Data byte 2 : 0VVV VVVV
* Channel is CCCC 
  * 10 for drum machines usually, 1 otherwise
  * subtract 1 as human written channels are CCCC + 1
* Middle C is 60 PPP PPPP
* mf is 64 VVV VVVV
* OFF is the same but with 1000 CCCC status byte

# Nucleo Notes

* USART3 is connected to the Nucleo USB cable
  * See UM1974 Rev 10 p 26 for details
  * On APB1 per Data Sheet
  * Alternate functions: See DataSheet Rev 8 p89 Table 13
    * USART3 TX is AF7 (Alternate Function 7)
  * Per Nucleo UM, USART3 TX = PD8; RX = PD9
    * SB4-7 control ST-LINK and/or morpho; by default it is both (UM1974 Rev 10 Sec 6.9 p 26)
  * PD10-12 are CK, CTS, RTS as well

* User LEDs LD1-3 are connected to: 
  * LD1: Green, PB0
  * LD2: Blue, PB7
  * LD3: Red: PB14
  * On when I/O is HIGH
  * See: UM1974 Rev 10 p 24
  
* B1 User push button
  * PC13
  * See: UM1974 Rev 10 p24
  
* USART for use for MIDI
  * See UM1974 Rev 10 p32 for the board pinout
  * See UM1974 Rev 10 p60-64 for details (Table 19)
  * I/Os are 3.3V (UM1974 Rev 10 p36)
  * USART_2, PD3-7, are on D51-55, connector CN9 pins 2-10 (even #s)
  * USART6, PG9 & 14, are on D0-1, connector CN10 pins 14,16
  

# Notes on expanding example to MIDI

* First, add the USART6 to the .ioc description
  * Pins PG9, PG14 become receive and transmit
  * Configure USART6 at 31,250 baud
  * When you save this, it will regenerate the main.c code
  