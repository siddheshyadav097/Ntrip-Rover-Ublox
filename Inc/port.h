#ifndef __PORT_H__
#define __PORT_H__




/*      Debug defines     */
#define DEBUG_UART                USART2
#define DEBUG_UART_CLK_ENABLE()   __USART2_CLK_ENABLE()
#define DEBUG_UART_CLK_DISABLE()  __USART2_CLK_ENABLE()
#define DEBUG_TX                  GPIO_PIN_2  //9
#define DEBUG_RX                  GPIO_PIN_3  //10
#define DEBUG_UART_AF             GPIO_AF7_USART2
#define DEBUG_UART_PORT           GPIOA
#define DEBUG_UART_IRQ            USART2_IRQn
#define DEBUG_IRQHandler          USART2_IRQHandler










#endif