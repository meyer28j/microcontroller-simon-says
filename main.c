
// HINT: when you hit a button, you get a signal
// can you take this information and drive the LED?
// i.e. is there a way to transform values directly
// from data rather than using code logic?

#include "main.h"
#include "STM32F103RB.h"

#include <time.h>

int globalCounter = 0;

#define GPIOC ((GPIO_TypeDef *) 0x40011000)
#define GPIOD ((GPIO_TypeDef *) 0x40011400)


// function taken from https://www.geeksforgeeks.org/time-delay-c/
void delay(uint32_t number_of_seconds)
{
    // Converting time into milli_seconds
    // uint32_t milli_seconds = 1000 * number_of_seconds;
		uint32_t milli_seconds = number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}

void blink(int count) {
		for (int i = 0; i < count; i++) {
		GPIOA->ODR |= (1u << 5);
		delay(1);
		GPIOA->ODR &= (1u << 5);
		delay(1);
	}
}

void button_control_LED_step() {
	// isolate input bit from GPIOC
	uint32_t input_bit = GPIOC->IDR;
	input_bit >>= 8; 		// shift bit 13 to 5th position
	input_bit &= (1u << 5); 	// isolate bit 5
	
	// isolate output bit from GPIOA
	uint32_t output_bit = GPIOA->ODR;
	output_bit &= (1u << 5); // isolate bit 5
	
	if (output_bit == input_bit) {

		// if they're the same, toggle bit 5 using XOR
		GPIOA->ODR ^= (1u << 5);
	
	}	// if not equal, do nothing
}


int main(void)
{
	
	// enable clock for ports A, B, C, D
	RCC->APB2ENR |= (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5); // set bits 2, 3, 4, 5 HIGH
	
	
	
	
	
	// enable 
	GPIOA->CRL |= (0x11u); // set MODE0 bits 0 and 1 HIGH
	
	// enable output-data pin 0 on port A
	GPIOA->ODR |= 0x1u; // set bit 0 to HIGH
	
	
	// enable port A5 as output (green LED on STM32F103RB board)
	// set port A MODE5 bits to 11 = output, 50MHz
	GPIOA->CRL |= (1u << 20) | (1u << 21); // set bits 20 and 21 HIGH
	
	// clear port A CNF5 bits to 00 = push-pull (in output mode)
	GPIOA->CRL &= ~(1u << 22) & ~(1u << 23); // clear bits 22 and 23
	
	
	// enable port D5 as input (black button)
	GPIOD->CRL &= ~(0x11u << 20); // clear bits 20 and 21 to LOW (input mode)
	// set bits 22 and 23 to 0x10... so just set bit 22 to HIGH (GPIO push/pull)
	GPIOD->CRL |= (0x1u << 23); 
	
	
	
	
	
	// enable user blue button control to toggle LED
	while (1) { // repeat indefinitely
	
		button_control_LED_step();
		
}
	
	return 0;
}
