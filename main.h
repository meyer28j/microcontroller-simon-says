#include <stdint.h>
#include "stm32f103rb.h"

void delay(uint32_t number_of_seconds);

void blink(int count);

void button_control_LED_step(void);

void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number);

void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number);

extern int globalCounter; 
