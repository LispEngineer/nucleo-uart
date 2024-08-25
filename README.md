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
* Windows 10
* STM32CubeIDE
* STM32 HAL libraries
* [UxMIDI tools](https://www.cme-pro.com/start-guide-for-uxmidi-tools-software-by-cme/)
* [MidiView](https://hautetechnique.com/midi/midiview/)
* [TM Terminal](https://marketplace.eclipse.org/content/tm-terminal) for Eclipse
* [Putty](https://www.putty.org/)
* Native Instruments FM8 (or really, any MIDI sound generator)

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
* DONE - Get UART transmit working for MIDI
  * Send a note on and off regularly
* Get UART receive working for MIDI
* Get DMA UART working
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
  

# Notes on expanding example to MIDI

* First, add the USART6 to the .ioc description
  * Pins PG9, PG14 become receive and transmit
  * Configure USART6 at 31,250 baud
  * When you save this, it will regenerate the main.c code
  
  
# Debugging Notes

## Lock up on serial auto-repeat

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
  
  
References:
* USART ISR - RM0410 Rev 5 p1291 34.8.8

Misc notes:
* Be sure to do a full power off or hardware reset before re-testing this
  as it seems not to reconfigure things with a soft-reset
* `UART_WaitOnFlagUntilTimeout` seems to check the overrun flag,
  but only if infinite delay is set.