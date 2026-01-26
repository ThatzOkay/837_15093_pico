#pragma once

#include "hardware/uart.h"

void process_cdc_port(int port, uint8_t* jvs_buff, uint32_t* offset_ptr);
void process_uart_port(uart_inst_t* port, uint8_t* jvs_buff, uint32_t* offset_ptr);