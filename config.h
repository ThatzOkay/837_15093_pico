#pragma once
#define LED_COUNT_01 64
#define LED_STRIP_01 16
#define LED_STRIP_01_OFFSET 0

#define LED_COUNT_02 64
#define LED_STRIP_02 15
#define LED_STRIP_02_OFFSET 0

#define DELAY 5
#define FORMAT PicoLed::FORMAT_RGB

#define MAX_PACKET 256

#define ENABLE_UART 1
#define UART0_ID uart0
#define UART0_TX_PIN 0
#define UART0_RX_PIN 1

#define UART1_ID uart1
#define UART1_TX_PIN 4
#define UART1_RX_PIN 5

#define BAUD_RATE 115200

#define ENABLE_DEBUG 1
#define ENABLE_LED_TEST 1
