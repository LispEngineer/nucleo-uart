/*
 * realmain.c
 * Since the main.c is auto-generated by STM32CubeIDE
 * this is the real "main" file for stuff that is not
 * auto-generated.
 *
 *  Created on: Aug 25, 2024
 *      Author: Douglas P. Fields, Jr.
 *   Copyright: 2024, Douglas P. Fields, Jr.
 *     License: Apache 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include "stm32f7xx_hal.h"
#include "string.h" // STM32 Core
#include "main.h"
#include "realmain.h"


#define WELCOME_MSG "Welcome to the Nucleo management console v3\r\n"
#define MAIN_MENU   "Select the option you are interested in:\r\n\t1. Toggle LD1 Green LED\r\n\t2. Read USER BUTTON status\r\n\t3. Clear screen and print this message\r\n\t4. Print counters"
#define PROMPT "\r\n> "
#define NOTE_ON  "\x90\x3C\x40"
#define NOTE_OFF "\x80\x3C\x40"

// From main.c
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;

static uint32_t overrun_errors = 0;
static uint32_t uart_error_callbacks = 0;
static uint32_t usart3_interrupts = 0;

void printWelcomeMessage(void) {
  /*
  HAL_UART_Transmit(&huart3, (uint8_t*)"\033[0;0H", strlen("\033[0;0H"), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart3, (uint8_t*)"\033[2J", strlen("\033[2J"), HAL_MAX_DELAY);
  */
  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n\r\n", strlen("\r\n\r\n"), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart3, (uint8_t*)WELCOME_MSG, strlen(WELCOME_MSG), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart3, (uint8_t*)MAIN_MENU, strlen(MAIN_MENU), HAL_MAX_DELAY);
}

uint8_t readUserInput(void) {
  char readBuf[1];
  HAL_StatusTypeDef retval;

  HAL_UART_Transmit(&huart3, (uint8_t*)PROMPT, strlen(PROMPT), HAL_MAX_DELAY);

#ifdef DO_UART_RECEIVE_TIMEOUTS
  // A low number makes it "work" (~50)
  // A high number is good for debugging (~10000)
  const uint32_t timeout = 10000; // Milliseconds (or timer ticks)

  do {
    retval = HAL_UART_Receive(&huart3, (uint8_t*)readBuf, 1, timeout);
    if (retval == HAL_TIMEOUT) {
      // This is here solely to enable a breakpoint on timeout.
      (*readBuf)++; // Increment the readBuf pointlessly, hopefully will not be optimized away
    }
  } while (retval == HAL_TIMEOUT);
#else
  retval = HAL_UART_Receive(&huart3, (uint8_t*)readBuf, 1, HAL_MAX_DELAY);
#endif // DO_UART_RECEIVE_TIMEOUTS

  return atoi(readBuf);
}


/* Just quickly send a midi note on and then off */
void send_midi_note_on_off(void) {
  const uint32_t delay = 100; // lower causes less likely overrun errors (e.g., 20)
  HAL_UART_Transmit(&huart6, (uint8_t *)NOTE_ON, 3, HAL_MAX_DELAY);
  HAL_Delay(delay);
  HAL_UART_Transmit(&huart6, (uint8_t *)NOTE_OFF, 3, HAL_MAX_DELAY);
}

uint8_t processUserInput(uint8_t opt) {
  char msg[30];

  send_midi_note_on_off();

  snprintf(msg, sizeof(msg), "%d", opt);
  HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

  switch (opt) {
  case 0:
    return 0;
  case 1:
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    break;
  case 2:
    snprintf(msg, sizeof(msg), "\r\nUSER BUTTON status: %s",
              HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) != GPIO_PIN_RESET ? "PRESSED" : "RELEASED");
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    break;
  case 3:
    return 2;
  case 4:
    snprintf(msg, sizeof(msg) - 1, "\r\nUA3I: %lu, ORE: %lu\r\n", usart3_interrupts, overrun_errors);
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    break;
  default:
    return 0;
  }

  return 1;
}

void realmain() {
  uint8_t opt = 0;
  uint32_t last_overrun_errors = overrun_errors;
  uint32_t last_usart3_interrupts = usart3_interrupts;
  char msg[36];

  printMessage:
  printWelcomeMessage();

  while (1) {
    opt = readUserInput();
    processUserInput(opt);

    if (overrun_errors != last_overrun_errors) {
      snprintf(msg, sizeof(msg) - 1, "\r\nORE: %lu\r\n", overrun_errors);
      last_overrun_errors = overrun_errors;
      HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }
    if (usart3_interrupts != last_usart3_interrupts) {
      snprintf(msg, sizeof(msg) - 1, "\r\nUA3I: %lu\r\n", usart3_interrupts);
      last_usart3_interrupts = usart3_interrupts;
      HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
    }

    if (opt == 3) {
      goto printMessage;
    }
  }
} // realmain()



////////////////////////////////////////////////////////////////////////////////////////
// Callbacks

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  // We just want to clear an overrun error and acknowledge it happened.
  // SEE: https://electronics.stackexchange.com/questions/376104/smt32-hal-uart-crash-on-possible-overrun
  // SEE: https://github.com/micropython/micropython/issues/3375
  uint32_t isrflags = READ_REG(huart->Instance->ISR);

  usart3_interrupts++;

  if( ((isrflags & USART_ISR_ORE) != RESET) && ((isrflags & USART_ISR_RXNE) == RESET) ) {
    __HAL_UART_CLEAR_IT(huart, UART_CLEAR_OREF); // This clears the ORE via the ICR (ORECF)
    huart->ErrorCode |= HAL_UART_ERROR_ORE; // Not sure what this does.

    uart_error_callbacks++;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
// HAL overrides


extern void UART_EndRxTransfer(UART_HandleTypeDef *huart);

/**
  * @brief  This function handles UART Communication Timeout. It waits
  *                  until a flag is no longer in the specified status.
  * @param huart     UART handle.
  * @param Flag      Specifies the UART flag to check
  * @param Status    The actual Flag status (SET or RESET)
  * @param Tickstart Tick start value
  * @param Timeout   Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef UART_WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status,
                                              uint32_t Tickstart, uint32_t Timeout)
{
  /* Wait until flag is set */
  while ((__HAL_UART_GET_FLAG(huart, Flag) ? SET : RESET) == Status) {

    // Check for overrun error & clear it, even if we're waiting forever.
    if ((READ_BIT(huart->Instance->CR1, USART_CR1_RE) != 0U) && (Flag != UART_FLAG_TXE) && (Flag != UART_FLAG_TC)) {
      if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) == SET) {
        /* Clear Overrun Error flag*/
        __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);

        /* Blocking error : transfer is aborted
        Set the UART state ready to be able to start again the process,
        Disable Rx Interrupts if ongoing */
        UART_EndRxTransfer(huart);

        huart->ErrorCode = HAL_UART_ERROR_ORE;

        /* Process Unlocked */
        __HAL_UNLOCK(huart);

        overrun_errors++;

        return HAL_ERROR;
      }
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((HAL_GetTick() - Tickstart) > Timeout) || (Timeout == 0U))
      {

        return HAL_TIMEOUT;
      }

      if ((READ_BIT(huart->Instance->CR1, USART_CR1_RE) != 0U) && (Flag != UART_FLAG_TXE) && (Flag != UART_FLAG_TC))
      {
        if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RTOF) == SET)
        {
          /* Clear Receiver Timeout flag*/
          __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_RTOF);

          /* Blocking error : transfer is aborted
          Set the UART state ready to be able to start again the process,
          Disable Rx Interrupts if ongoing */
          UART_EndRxTransfer(huart);

          huart->ErrorCode = HAL_UART_ERROR_RTO;

          /* Process Unlocked */
          __HAL_UNLOCK(huart);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  return HAL_OK;
}
