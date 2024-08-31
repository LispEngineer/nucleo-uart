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
* [Arm Cortex-M7 Technical Reference Manual](https://developer.arm.com/documentation/ddi0489/latest/)
* [Arm v7-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0403/latest/)
* [MIDI 1.0](https://midi.org/midi-1-0)
* [MIDI 1.0 Core Specifications](https://midi.org/midi-1-0-core-specifications)
* [MIDI 1.0 MPE](https://midi.org/midi-1-0-detailed-specification)

Misc Docs:
* [STM32 gotchas](http://efton.sk/STM32/gotcha/index.html)

Goals:
* DONE - Get UART transfer working for ST-LINK console
* DONE - Get UART transmit working for MIDI
  * Send a note on and off regularly
* DONE - Get UART receive working for MIDI
  * This was implemented using LL instead of HAL (seems much nicer)
* DONE - Make an event-driven non-DMA, non-interrupt-driven MIDI & USB Serial
  version of the code with circular output buffers
* Clean up the code
* Migrate from HAL to LL for UARTs
* Get DMA receive & send version working
* Build something simple:
  * MIDI receive
  * Text translation of MIDI over Serial
  * MIDI retransmit (e.g., a THRU with a bit of latency)
  * Measure latency
  * Minimize latency

# BUGS!

* If you do typing autorepeat into the Serial console, the board hangs!
  
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