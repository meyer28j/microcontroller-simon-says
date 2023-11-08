#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "STM32F103RB.h"

void delay(uint32_t volatile delay_value);

void blink(int led, uint32_t duration);

int detect_input(void);

void detect_input_global(int trigger);

int timer_interruptable(uint32_t max_time);

void write_input_to_output(GPIO_TypeDef* GPIO_in, 
													GPIO_TypeDef* GPIO_out, 
													int pin_in, 
													int pin_out);
													
void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number);

void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number);
													
void initialize_ports(void);

extern int globalCounter; 
													
extern int input_global;
													

