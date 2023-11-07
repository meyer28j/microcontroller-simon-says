#include <stdint.h>
#include "STM32F103RB.h"

void delay(uint32_t number_of_seconds);

void blink(int count);

void write_input_to_output(GPIO_TypeDef* GPIO_in, 
													GPIO_TypeDef* GPIO_out, 
													int pin_in, 
													int pin_out);
													
void button_control_LED_step(void);

void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number);

void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number);

extern int globalCounter; 
