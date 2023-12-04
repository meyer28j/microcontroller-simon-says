#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "STM32F103RB.h"

void delay(uint32_t volatile delay_value);

void led_on(int led_number);

void led_off(int led_number);

void blink(int led, uint32_t duration);

void blink_multi(int led_number[], int array_size, uint32_t duration);

int detect_input(void);

void detect_input_global(int trigger);

int timer_button_interrupt(uint32_t volatile max_time);

int timer_button_interrupt_with_seeding(uint32_t volatile max_time);
								
void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number);

void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number);
													
void initialize_ports(void);
													
void 	initialize_leds_buttons(void);
													
void display_knight_rider(uint32_t blink_speed);
													
void display_binary(int score);


extern int seed_counter; 
													
extern int input_global;
													

