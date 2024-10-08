# Nucleo MIDI Test

* Author: [Douglas P. Fields, Jr.](mailto:symbolics@lisp.engineer)
* Copyright 2024, Douglas P. Fields Jr.
* License: [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)
* Started: 2024-08-25

Credits:
* [C Ring Buffer](https://github.com/AndersKaloer/Ring-Buffer)
  * Copyright (c) 2014 Anders Kalør
  * License: MIT
  * See: `ringbuffer.c` and `ringbuffer.h`

Hardware:
* Board: [ST Nucleo-F767ZI](https://www.st.com/en/evaluation-tools/nucleo-f767zi.html)
* MIDI: [ubld.it MIDI-Mini Breakout Board](https://ubld.it/midi-mini)
* [CME U2MIDI PRO](https://www.cme-pro.com/u2midi-pro-usb-to-midi-cable/)
  USB MIDI interface

Software:
* Windows 10
* STM32CubeIDE
* STM32 HAL libraries
* [UxMIDI tools](https://www.cme-pro.com/start-guide-for-uxmidi-tools-software-by-cme/)
* [MidiView](https://hautetechnique.com/midi/midiview/)
  * Also see [ShowMIDI](https://github.com/gbevin/ShowMIDI)
* [TM Terminal](https://marketplace.eclipse.org/content/tm-terminal) for Eclipse
* [Putty](https://www.putty.org/)
* Native Instruments FM8 (or really, any MIDI sound generator)
* [SendMIDI](https://github.com/gbevin/SendMIDI)

Documentation:
* [STM32F7 HAL, UM1905](https://www.st.com/resource/en/user_manual/um1905-description-of-stm32f7-hal-and-lowlayer-drivers-stmicroelectronics.pdf)
* [STM32 Cube IDE User Guide, UM2609](https://www.st.com/resource/en/user_manual/um2609-stm32cubeide-user-guide-stmicroelectronics.pdf)
* [STM32 Cube MX User GUide, UM1718](https://www.st.com/resource/en/user_manual/um1718-stm32cubemx-for-stm32-configuration-and-initialization-c-code-generation-stmicroelectronics.pdf)
* [STM32F767xx Datasheet](https://www.st.com/resource/en/datasheet/stm32f765bi.pdf)
* [RM0410 STM32F76 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0410-stm32f76xxx-and-stm32f77xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
* [UM1974 Nucleo-144 User Manual](https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf)
* [AN4667 STM32F7 Series system architecture and performance](https://www.st.com/resource/en/application_note/an4667-stm32f7-series-system-architecture-and-performance-stmicroelectronics.pdf)
* [AN4661 Getting started with STM32F7 Series MCU hardware development](https://www.st.com/resource/en/application_note/an4661-getting-started-with-stm32f7-series-mcu-hardware-development-stmicroelectronics.pdf)
* [AN4839 Level 1 cache on STM32F7 Series...](https://www.st.com/resource/en/application_note/an4839-level-1-cache-on-stm32f7-series-and-stm32h7-series-stmicroelectronics.pdf)
* [Arm Cortex-M7 Technical Reference Manual](https://developer.arm.com/documentation/ddi0489/latest/)
* [Arm v7-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0403/latest/)
* [MIDI 1.0](https://midi.org/midi-1-0)
* [MIDI 1.0 Core Specifications](https://midi.org/midi-1-0-core-specifications)
* [MIDI 1.0 MPE](https://midi.org/midi-1-0-detailed-specification)

Misc Docs:
* [STM32 gotchas](http://efton.sk/STM32/gotcha/index.html)
* [Interrupt by Memfault - .noinit](https://interrupt.memfault.com/blog/noinit-memory)
  * This is a good embedded blog in general

Goals:
* DONE - Get UART transfer working for ST-LINK console
* DONE - Get UART transmit working for MIDI
  * Send a note on and off regularly
* DONE - Get UART receive working for MIDI
  * This was implemented using LL instead of HAL (seems much nicer)
* DONE - Make an event-driven non-DMA, non-interrupt-driven MIDI & USB Serial
  version of the code with circular output buffers
  * This operates at 114kHz: ~114 times through the main loop per millisecond
    * This was at 96MHz HCLK
    * Using external ST-Link HSE at 8 MHz
  * It operates at 212-213kHz at 216 MHz (current configuration)
    * The scaling was sub-linear, perhaps because we are running up against other speed limitations.
    * 2.25x Cortex clock, 1.87x Performance increase (15/8ths almost)
  * For argument's sake, I set it to 192 MHz, giving 202-203 kHz through the loop
    * 2x Cortex clock, slower APB1/2 clocks (54 -> 48, 108 -> 96), for 1.77x performance
  * Penultimate Argument: 48MHz, but all busses also at that speed: 70-71kHz through the loop
    * 0.5x system clock speed, 0.61x loop performance.
  * Final Argument: 54MHz, all busses also at that speed: 79-80kHz
    * 1.13x clock speed; 1.13x loop speed
    * This implies about 675 clock cycles per loop
    * 56% of the Cortex clock speed, but 69% of the loop speed: pretty good trade
    * **We will leave things at this speed for now**
* Optimize memory usage for various RAM speed areas
  * (This is for fun; don't need optimization for realz yet)
  * DONE - Move stack to DTCM for improved performance
  * DONE - Enable DTCM allocation of some variables
    * DONE - BSS (zero-initialized, requires startup changes) - .fast_bss
    * DONE - Pre-initialized variables (copied from ROM, requires startup changes) - .fast_data
    * NOT DOING - Non-initialized variables
  * See if we can get GCC to auto allocate something to .fast_bss or .fast_data
    depending on if the variable is initialized
* DONE - Figure out a way to detect stack overflow - but hopefully it will just
  error into infinite loop with Default Handler
  * [Cortex-M7 Fault Handling](https://developer.arm.com/documentation/ddi0489/f/memory-system/fault-handling)
  * Running in debugger, it halts at `Hardfault_Handler`, which just infinite loops
    as expected.
* DONE - Get I2S output audio working
  * With DMA
  * Turns on the red LED whenever it is filling the DMA buffer
* DONE - Get Simple Tone Generator working with I2S DMA audio
* DONE - Get simple MIDI monophonic synth running
* Clean up the code
* Migrate from HAL to LL for UARTs
* Build something simple:
  * MIDI receive
    * Parse MIDI into full messages
  * Text translation of MIDI over Serial
  * MIDI retransmit (e.g., a THRU with a bit of latency)
  * Measure latency
  * Minimize latency
* MIDI thru (no real reason)
	* Send MIDI out as fast as it comes in
 	* Measure latency difference
 	* Attempt to get latency difference < 3 bits after receiving a byte, so 13 bits
 	  * Measure latency from the start bit of input to start bit of output
 	* 13 bits at 31.5kbps = 4.13 x 10^-4 = 413µs; under 1ms 
* Connect small I2C/SPI monitor
  * Display MIDI traffic
  
# Memory Configuration

I am trying to make it so I can put stack in the DTCM.

Linker Symbols, and where they are used:
* _estack
  * _sbrk
* _Min_Heap_Size
* _Min_Stack_Size
  * _sbrk
* _etext
* _exidx_start, _end
* __preinit_array_start, _end
* __init_array_start, _end
* __fini_array_start, _end
* _sidata
* _sdata
* _edata
* _sbss, __bss_start__: start of .bss
* _ebss, __bss_end__: End of .bss
* end, _end: end of .bss (after alignment) and start of ._user_heap_stack
  * _sbrk

Speeds:
* Using RAM as SRAM1 only: LPT 72-73
* Using RAM as DTCM  only: LPT 84-85
* Using RAM as SRAM2 only: LPT 70-71
* Using DTCM for Stack & SRAM1 for everything else: LPT 79

# Hardware Configuration

* ST-Link USB/Serial COM port
  * 460,800 bps, 8-N-1
  * Uses USART3 of the STM32
* MIDI
  * USART6 of the STM32
  * D0 & D1 pins of the NUCLEO
  * 3V3 power
  
# Toolchain Info

Location: `C:\Apps\STM32CubeIDE_1.16.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.win32_1.0.200.202406191623\tools\bin`

* STM32CubeIDE 1.16.0
  * GNU GCC 12.3.1 [docs](https://gcc.gnu.org/onlinedocs/12.3.0/)
  * GNU Binutils 2.40 [docs](https://sourceware.org/binutils/docs-2.40/)

# BUGS!

(none now)
  
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
* ON Status byte : 1001 CCCC -> 1001 0000 -> 0x90
* Data byte 1    : 0PPP PPPP ->        60 -> 0x3C
* Data byte 2    : 0VVV VVVV ->        64 -> 0x40
* Channel is CCCC 
  * 10 for drum machines usually, 1 otherwise
  * subtract 1 as human written channels are CCCC + 1
* Middle C is 60 PPP PPPP
* mf is 64 VVV VVVV
* OFF is the same but with 1000 CCCC status byte -> 0x80

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
  * There seems to be no other USARTs available than 2 & 6 (and 3 for serial USB)
  
* UART/USART counts: 4/4
  * DS11532 Rev 8 Table 2 p17 for STM32F767Zx 
 
* USART Alternate Function Mappings per DS11532 Rev 8 p89+ for STM32F767Zx
* By U(S)ART:
  * PA8-12:  AF7, USART1
  * PB6-7:   AF7, USART1
  * PB14-15: AF4, USART1
  * PA0-4:   AF7, USART2
  * PD3-7:   AF7, USART2
  * PD3-7:   AF7, USART2
  * PB10-14: AF7, USART3
  * PC10-12: AF7, USART3
  * PD8-12:  AF7, USART3
  * PA0-1:   AF8, UART4
  * PA11-12: AF6, UART4
  * PA15:    AF8, UART4
  * PB0:     AF8, UART4
  * PB14-15: AF8, UART4
  * PC10-11: AF8, UART4
  * PD0-1:   AF8, UART4
  * PC11:    AF8, UART4
  * PD0-1:   AF8, UART4
  * PH13-14: AF8, UART4
  * PI9:     AF8, UART4
  * PB5-6:   AF1, UART5
  * PB8-9:   AF7, UART5
  * PB12-13: AF8, UART5
  * PC8-9:   AF7, UART5 cts/rts
  * PC12:    AF8, UART5
  * PD2:     AF8, UART5
  * PC12:    AF8, UART5 TX
  * PD2:     AF8, UART5 RX
  * PC6-8:   AF8, USART6
  * PG7-9:   AF8, USART6
  * PG12-15: AF8, USART6
  * PA8:    AF12, UART7 RX
  * PA15:   AF12, UART7 TX
  * PB3-4:  AF12, UART7
  * PE7-10:  AF8, UART7
  * PF6-9:   AF8, UART7
  * PD14-15: AF8, UART8
  * PE0-1:   AF8, UART8
* By PIN:
  * PA0-1:   AF8, UART4
  * PA0-4:   AF7, USART2
  * PA8:    AF12, UART7 RX
  * PA8-12:  AF7, USART1
  * PA11-12: AF6, UART4
  * PA15:    AF8, UART4
  * PA15:   AF12, UART7 TX
  * PB0:     AF8, UART4
  * PB3-4:  AF12, UART7
  * PB5-6:   AF1, UART5
  * PB6-7:   AF7, USART1
  * PB8-9:   AF7, UART5
  * PB10-14: AF7, USART3
  * PB12-13: AF8, UART5
  * PB14-15: AF4, USART1
  * PB14-15: AF8, UART4
  * PC6-8:   AF8, USART6
  * PC8-9:   AF7, UART5 cts/rts
  * PC10-11: AF8, UART4
  * PC10-12: AF7, USART3
  * PC11:    AF8, UART4
  * PC12:    AF8, UART5
  * PC12:    AF8, UART5 TX
  * PD0-1:   AF8, UART4
  * PD2:     AF8, UART5 RX
  * PD3-7:   AF7, USART2
  * PD8-12:  AF7, USART3
  * PD14-15: AF8, UART8
  * PE0-1:   AF8, UART8
  * PE7-10:  AF8, UART7
  * PF6-9:   AF8, UART7
  * PG7-9:   AF8, USART6
  * PG12-15: AF8, USART6
  * PH13-14: AF8, UART4
  * PI9:     AF8, UART4

* NUCLEO-F767ZI pins for U(S)ARTs:
  * UM1974 Rev 10 Table 19 p60+
  * By Nucleo Pin: ------------------
  * A0     = PA3     = USART2, UART4
  * A4-5   = PB9-8   = USART1, UART5 (Check Solder Bridges)
  * D0     = PG9     = USART6
  * D1     = PG14    = USART6
  * D6     = PE9     = UART7
  * D10-9  = PD14-15 = UART8
  * D15-14 = PB8-9   = UART5
  * D16    = PC6     = USART6
  * D17    = PB15    = USART1, UART4
  * D18    = PB13*   = USART3, UART5  (* JP7)
  * D19    = PB12    = USART3, UART5
  * D20    = PA15    = UART4, UART7
  * D21    = PC7     = USART6, UART5
  * D22    = PB5     = UART5
  * D23    = PB3     = UART7
  * D24    = PA4     = USART2
  * D25    = PB4     = UART7
  * D26    = PB6     = USART1, UART5
  * D29-30 = PD12-11 = USART3
  * D32    = PA0     = USART2, UART4
  * D33    = PB0     = UART4
  * D34    = PE0     = UART8
  * D36-35 = PB10-11 = USART3
  * D40    = PE10    = UART7
  * D41-42 = PE7-8   = UART7
  * D45-47 = PC10-12 = USART3, UART4
  * D48    = PD2     = UART5
  * D51-55 = PD7-3   = USART2
  * D61-63 = PF7-9*  = USART7
  * D67-66 = PD0-1   = UART4
  * By STM32 Pin: ----------------
  * D32    = PA0     = USART2 CTS, UART4 TX
  * A0     = PA3     = USART2 RX
  * D24    = PA4     = USART2 CK
  * D20    = PA15    = UART4 RTS, UART7 TX
  * D33    = PB0     = UART4 CTS
  * D23    = PB3     = UART7 RX
  * D25    = PB4     = UART7 TX
  * D22    = PB5     = UART5 RX
  * D26    = PB6     = USART1 TX, UART5 TX
  * D15-14 = PB8-9   = UART5 RX TX
  * A4-5   = PB9-8   = UART5 RX TX (Check Solder Bridges)
  * D36-35 = PB10-11 = USART3 RX TX
  * D19    = PB12    = USART3 CK, UART5 RX
  * D18    = PB13*   = USART3 CTS, UART5 TX (* JP7)
  * D17    = PB15    = USART1 RX, UART4 CTS
  * D16    = PC6     = USART6 TX
  * D21    = PC7     = USART6 RX
  * D45-47 = PC10-12 = USART3 TX RX CK, UART4 TX RX, UART5 TX
  * D67-66 = PD0-1   = UART4 RX TX
  * D48    = PD2     = UART5 RX
  * D51-55 = PD7-3   = USART2 CTS RTS TX RX CK
  * D29-30 = PD12-11 = USART3 CTS RTS
  * D10-9  = PD14-15 = UART8 CTS RTS
  * D34    = PE0     = UART8 RX
  * D41-42 = PE7-8   = UART7 RX TX
  * D6     = PE9     = UART7 RTS
  * D40    = PE10    = UART7 CTS
  * D61-63 = PF7-9*  = UART7 TX RTS CTS
  * D0     = PG9     = USART6 RX
  * D1     = PG14    = USART6 TX
  * By U(S)ART, TX/RX only: ----------------
  * D26    = PB6     = USART1 TX, UART5 TX
  * D17    = PB15    = USART1 RX, UART4 CTS
  * A0     = PA3     = USART2 RX
  * D51-55 = PD7-3   = USART2 CTS RTS TX RX CK
  * D36-35 = PB10-11 = USART3 RX TX
  * D45-47 = PC10-12 = USART3 TX RX CK, UART4 TX RX, UART5 TX
  * D32    = PA0     = USART2 CTS, UART4 TX
  * D67-66 = PD0-1   = UART4 RX TX
  * D45-47 = PC10-12 = USART3 TX RX CK, UART4 TX RX, UART5 TX
  * D26    = PB6     = USART1 TX, UART5 TX
  * D22    = PB5     = UART5 RX
  * D15-14 = PB8-9   = UART5 RX TX
  * A4-5   = PB9-8   = UART5 RX TX (Check Solder Bridges)
  * D45-47 = PC10-12 = USART3 TX RX CK, UART4 TX RX, UART5 TX
  * D48    = PD2     = UART5 RX
  * D19    = PB12    = USART3 CK, UART5 RX
  * D18    = PB13*   = USART3 CTS, UART5 TX (* JP7)
  * D16    = PC6     = USART6 TX
  * D21    = PC7     = USART6 RX
  * D0     = PG9     = USART6 RX
  * D1     = PG14    = USART6 TX
  * D20    = PA15    = UART4 RTS, UART7 TX
  * D41-42 = PE7-8   = UART7 RX TX
  * D61-63 = PF7-9*  = UART7 TX RTS CTS
  * D23    = PB3     = UART7 RX
  * D25    = PB4     = UART7 TX
  * D34    = PE0     = UART8 RX
  * ------------------
  * Seems like UART8 has RX only?

# Notes on expanding example to MIDI

* First, add the USART6 to the .ioc description
  * Pins PG9, PG14 become receive and transmit
  * Configure USART6 at 31,250 baud
  * When you save this, it will regenerate the main.c code
  
* To send MIDI with Windows:
  * `.\sendmidi.exe dev "U2MIDI Pro" on 69 96`
  
  
# Debugging Notes

## [FIXED] Lock up on serial auto-repeat

* Run the application with ST-Link connected using the STM32CubeIDE
  (Eclipse) Debug option
  * Hit F8/Resume to run the application full speed
* Note application works normally with modest rate key presses
  * MIDI is output, and the notes sound in an attached MIDI device
* Hold down a key in terminal for auto-repeat
* Note that after a short time, the application stops responding
* "Suspend" the application when it is stuck
* Note call stack: `readUserInput()` -> `HAL_UART_Receive` ->
  `UART_WaitOnFlagUntilTimeout`
* Note that it is infinitely looping in the `UART_WaitOnFlagUntilTimeout`
  method on the check `while ((__HAL_UART_GET_FLAG(huart, Flag) ? SET : RESET) == Status)`

### Idea 1 - Workaround
 
* Add a timeout so that it will stop the receive when it infinitely loops
* This seems to work

### Idea 2 - Figure out the problem and fix it

* Hypothesis:
  * When you receive a new byte at the UART before the current one
    has been read by polling, an error (overrun?) occurs on the UART
  * The STM32 HAL code is not handling this situation properly, or
    resetting the overrun error
* Investigation:
  * `Advanced Features` on the UART configuration in the `.ioc` file
    shows that `Overrun` is `Enable`d
  * Let's `Disable` it on `USART6` and see what happens
  * (This requires regenerating code.)
* Disabling Overrun detection "works"
* Discovered [this post](https://community.st.com/t5/stm32-mcus-embedded-software/stm32cube-uart-polling-parity-overrun-errors-checks-missing/td-p/128774)
  which talks about it (although not in a particularly friend manner)
  
References discovered during investigation:
* [Tilen Majerle](https://docs.majerle.eu/en/latest/) - has DMA UART example
  * [The DMA UART example](https://github.com/MaJerle/stm32-usart-uart-dma-rx-tx)
* Discussions
  * [UART overrun](https://community.st.com/t5/stm32-mcus-products/is-the-uart-overrun-unavoidable/td-p/215016)
  * [Polling receive](https://community.st.com/t5/stm32-mcus-embedded-software/how-do-i-receive-a-command-through-uart-in-polling-mode/td-p/276249)
* Misc
  * [MIDIbox DMA](http://www.ucapps.de/mbhp_core_lpc17.html)  
  * [Another DMA UART example](https://github.com/electricui/stm32-dma-uart-eui)
  
### Idea 3 - Poll but do overrun detection and reset

Try 1:
* Hypothesis:
  * Re-enable the overrun detection
  * Assume it sends an interrupt on overrun
  * Clear the overrun flag in an error callback `HAL_UART_ErrorCallback` we define
* Results:
  * Still crashes
  * Still loops infinitely
  * The error callback is not called - perhaps error interrupts are not enabled
  * The ISR is 110 0000  0001 0000  1101 1000
    * ORE is bit 3: 1. Overrun error (RM0410 Rev 5 p1294)
    * RXNE is bit 5: 0.
  * The ICR (Interrupt flag clear register) is 0
    * ORECF is bit 3, Overrun error clear flag
    
Try 2:
* Hypothesis: `UART_WaitOnFlagUntilTimeout` should check for overrun
  even when it is NOT checking for timeouts
  * Keep overrun detection enabled (re-enable it)
* Implementation:
  * Flag original `UART_WaitOnFlagUntilTimeout` as `__weak`
  * Export `UART_EndRxTransfer` (make not `static`)
  * Reimplement `UART_WaitOnFlagUntilTimeout`
  * Count overrun errors
  * Print the count when overrun errors happens
  * Increase the note on to off time to 100ms from 20ms
* Results:
  * Overrun errors occur reliably
  * They don't break anything
  * They are reported via serial out


Try 3 (TODO):
* Hypothesis: Enable interrupts on errors and so ORE can be cleared via interrupt
* Implementation:
  * Implement `USART3_IRQHandler()`
  * In the `.ioc` file configuration, for `NVIC`, enable `USART3 global interrupt`
    * Set the priority above `0`; I used `2`
  * When you save the `.ioc` file, it will rebuild the C source, wiping out the
    changes we made to `Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c`ddi0403
    in try 2. So, `git checkout -- Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c`
    to undo the edits after making sure nothing else changed.
  * Note: code generated by `.ioc` writes the `USART3_IRQHandler()` into `stm32f7xx_it.c`,
    and not as `__weak` either. This just calls `HAL_UART_IRQHandler(&huart3)`.
  * This calls the `HAL_UART_ErrorCallback` we defined earlier that never got called.
    So we don't need to define `USART3_IRQHandler()` after all. (Removed it)
* Results
  * Everything still "works" but no interrupts get called.
* Notes:
  * `UART_IT_ERR`
  * See `USART_CR3` bit `EIE` on RM0410 Rev 5 p1287
  * See `__HAL_UART_ENABLE_IT` macro in `stm32f7xx_hal_uart.h`
  * This should probably be set in `HAL_UART_MspInit()` which is defined
    in `stm32f7xx_hal_msp.c`, a generated file
    * Enter it into the code block `USER CODE BEGIN USART3_MspInit 1`
* Implementation 2:
  * As per notes above: enable the interrupt
  * Also add it to the De-Init function to disable that interrupt
* Results 2:
  * The interrupt occurs exactly once.
  * Maybe the interrupt enable has to be re-enabled each time
    it fires an interrupt?
* Implementation 3:
  * Re-enable the interrupts while/after they are firing
    * In `HAL_UART_ErrorCallback`
    * call `__HAL_UART_ENABLE_IT(huart, UART_IT_ERR);`
    * If this is done in the check for overrun, it doesn't work, as that check never passes.
      * TODO: Fix ^^^
* Results 3:
  * It now gets UART3 interrupts
  * It doesn't get any EC counters
* Implementation 4:
  * HAL interrupt code already checks & clears the ORE,
    but it marks the ErrorCode which we can check later
    in the ErrorCallback.
* Results 4:
  * Works correctly!!
* Implementation 5: Interrupt OverRunErrors without HAL receive changes
  * Hypothesis: the edit to `UART_WaitOnFlagUntilTimeout` is no longer needed
    now that we handle ORE with interrupts.
  * `ifdef` out my `USE_DPF_WaitOnFlagUntilTimeout` so the `__weak` one will be back.
* Results 5:
  * Works, same as #4
  
References:
* USART ISR - RM0410 Rev 5 p1291 34.8.8

Misc notes:
* Be sure to do a full power off or hardware reset before re-testing this
  as it seems not to reconfigure things with a soft-reset
* `UART_WaitOnFlagUntilTimeout` seems to check the overrun flag,
  but only if infinite delay is set.
  
## [FIXED] Must manually enable interrupts for USART6

For some reason, while the HAL properly enables interrupts automatically
for USART3 (ST-Link UART), it doesn't do the same for USART6 despite,
as far as I can tell, their being configured identically (save for
baud rate). I had to manually call this once:

* `LL_USART_EnableIT_ERROR(huart6.Instance);`

So, the problem is, you have to enable interrupts in the `HAL_UART_MspInit()`
manually at the start:

* `__HAL_UART_ENABLE_IT(huart, UART_IT_ERR);`

I had done this for USART3, but forgot to do it for USART6.

# Random Thoughts

## Adding external SDRAM

* 12 address pins (DS11532 Rev 8 p86), 32 data pins, 2 bank pins
* RM0410 Rev 5 Section 13.8
* AN4667 Rev 4 Section 1.5.3 p17 - remap to make the SDRAM cacheable

## Optimizing main loop

* Move hot code into ITCM-RAM (Instruction RAM, 16Kbytes)
  * DS11532 Rev 8 Section 3.5 p22
  * RM0410 Rev 5 Section 2.3 p81
  * AN4667 Rev 4 Section 1.5.2 p12
  
* Move hot data into DTCM-RAM (128 Kbytes)
  * Stack
  * Circular I/O buffers
  * Memory Map: RM0410 Rev 5 Figure 2 p77
    * 0x0000 0000 - 0x0000 3FFF : ITCM RAM  16 kB
    * 0x2000 0000 - 0x2001 FFFF : DTCM RAM 128 kB 
    * 0x2002 0000 - 0x2007 BFFF : SRAM1    368 kB
    * 0x2007 C000 - 0x2007 FFFF : SRAM2     16 kB
  * May not matter; there is 16kB of I/D cache
  
* TCM RAM can do double word access (RM0410 Rev 5 p81)

* See tips in AN4667 Rev 4 Section 5.2, e.g.:
  * "...access the Flash memory through the AXI/AHB interface instead of the
    TCM to take advantage of the performance induced by the cache size."
  * "...recommended to disable the read-modify-write of the DTCM-RAM in the DTCM interface (in
    the DTCMCR register) to increase the performance."

## Optimizing the circular buffers

## Making my own board

Docs:
* See AN4661

Examples: 
* [Example Board](https://github.com/pms67/LeDSP-Audio-SoM)
* Example: Ksoloti
* Example: Nucleo designs/schmetics
* Pimoroni pico audio board

# Audio Output

Docs:
* [Pimoroni Pico Audio Pack PIM544](https://shop.pimoroni.com/products/pico-audio-pack?variant=32369490853971)
* [PCM5102A I2S DAC](https://www.ti.com/product/PCM5102A)
* [PCM5102 and STM32](http://elastic-notes.blogspot.com/2020/11/use-pcm5102-with-stm32_76.html)
* [Wikipedia](https://en.wikipedia.org/wiki/I%C2%B2S)
* RM0410 Rev 5 Chapter 35
  * Section 35.7
* UM1905 Rev 5 Chapter 35 (HAL I2S Driver)
* Diodes PAM8901/8908 headphone amplifier
* Diodes AP7333 linear regulator (300mA)

Nomenclature:
* STM32: SD (serial data) = I2S DIN/DATA (Audio data in, data)
* STM32: WS (word select) = I2S LRCLK (audio data word left/right clock)
* STM32: CK (serial clock) = I2S BCK (audio data bit clock)
* STM32: MCK (master clock) - not used

STM32 Configuration:
* I2S1 - Nucleo-F767ZI
	* PA4 I2S1_WS - CN7-15 ; D24
	* PA5 I2S1_CK - CN7-10 ; D13
	* PD7 I2S1_SD - CN9-2  ; D51
* I2S2 - Nucleo-F767ZI - I could not get this one to work for the life of me
  * PB10	I2S2_CK - CN10-32 ; D36   - Purple
  * PB12	I2S2_WS - CN7-7   ; D19   - Gray
  * PC3	  I2S2_SD - CN9-5   ; A2    - Blue
* I2S3 - this one works
  * PA4  I2S3_WS - CN7-17  ; D24 - Gray
  * PB2  I2S3_SD - CN10-15 ; D27 - Blue
  * PC10 I2S3_CK - CN8-6   ; D45 - Purple
  
Getting this working:
* I tried to get it working using I2S2, but it failed.
	* I could never get any clock outputs
* After several hours, I gave up and added I2S3
  * This immediately worked
* So I removed I2S2
  * And then it broke again
* I re-added it and noticed that adding I2S3 AND I2S2 added another section:
  * `void PeriphCommonClock_Config(void);`
  
This was removed:  
```
/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  PeriphClkInitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}
```

But in `HAL_I2S_MspInit()` this was added, that seems NOT to work:

```
  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
    PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
    PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
    PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
    PeriphClkInitStruct.PLLI2SDivQ = 1;
    PeriphClkInitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
```

The obvious difference is that the 
`PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;`
statement is missing in the broken code. Adding that back in manually
makes it work again!

## Notes on configuration

* Configuration in common:
  * 32 kHz
  * Mode Master Transmit
  * I2S Philips
* Varying things:
	* 16 bits data on 16 bits frame - very loud! Even with low gain.
	* 16 bits data on 32 bits frame - quiet, even with high gain
* Frequency up to 48 kHz with 16 / 16
  * Higher frequency for the same waveform (as expected)
  * When going to 16 / 32 - very low volume and odd sound
  
  
## Next steps

* Figure out what the framing thing is all about
* DONE - Get DMA working

## Notes on PCM5102A

* SCK (master clock) is not necessary - 3 wires suffice
* XSMT pin - soft mutes after 104 samples when low
* Zero data detect - mutes when receiving all 0's
* LOW = left, HIGH = Right on LRCK
* MSB-first, 16, 24, 32 bits data, framing at 32, 48, 64x Fs
* Board layout should widely separate analog and digital signals
  * SLAS859C Revised May 2015 section 12
* 8 kHz to 384 kHz

## Notes on STM32

* CHSIDE bit in SPIx_SR register
  * Left always sent first, followed by right
  * Refreshed when TXE goes high
* TXE - transmit buffer empty, next data ready to be transmitted
* Sends 16 bits at a time

Error Flags:
* UDR - underrun - cleared by read on SPIx_SR
  * Interrupt generation is possible
  
Speeds required
* 32kHz @ 16bit x 2 channels = 64kHz (yikes)
  * Doing this by polling may not be plausible

## DMA Audio

* [Phil's Lab DMA Video](https://www.youtube.com/watch?v=zlGSxZGwj-E)

Process:
* Set up a DMA to be in a circular queue, so it always
  loops through a buffer continuously sending
* Set up interrupts to get notified of the state of the DMA
  * Half done, all done
  * (Remember it auto restarts when all done)
* When half done, re-fill the first half of the DMA send buffer
* When all done, re-fill the second half of the DMA send buffer

TODO:
* Figure out what to do about race conditions like, if we get
  an interrupt while we're filling up the buffer
