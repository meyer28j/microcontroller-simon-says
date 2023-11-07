
// HINT: when you hit a button, you get a signal
// can you take this information and drive the LED?
// i.e. is there a way to transform values directly
// from data rather than using code logic?

#include "main.h"
//#include "STM32F103RB.h"

#include <time.h>


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

void write_input_to_output(GPIO_TypeDef* GPIO_in, 
													GPIO_TypeDef* GPIO_out, 
													int pin_in, 
													int pin_out) {
	
	// isolate input bit from GPIO_in
	uint32_t input_bit = GPIO_in->IDR;
	input_bit >>= (pin_in - pin_out); 	// shift input bit to output bit position
	input_bit &= (1u << pin_out); 			// isolate input bit in output bit position
	
	// isolate output bit from GPIO_out
	uint32_t output_bit = GPIO_out->ODR;
	output_bit &= (1u << pin_out); // isolate output bit
													
	if (output_bit == input_bit) {

		// toggle output bit using XOR
		GPIO_out->ODR ^= (1u << pin_out);
	
	}	// if not equal, do nothing
}

void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number) {
	// enable a specified port as output, 50MHz push-pull
	// set port MODEx bits to 11 = output, 50MHz
	// clear port CNFx bits to 00 = push-pull (in output mode)
	
	// convert port_number to half-byte position
	// used for shifting bit set/clear masks
	int pin_bit_address = port_number << 2;
	
	// distinguish between CRL (ports 0-7) and CRH (ports 8-15)
	if (port_number < 8) {
		
		// set MODE bits HIGH, 11 = output, 50MHz
		GPIO->CRL |= (1u << pin_bit_address) | (1u << (pin_bit_address + 1)); 

		// set CNF bits LOW, 00 = push-pull (in output mode)
		GPIO->CRL &= ~(1u << (pin_bit_address + 2)) & ~(1u << (pin_bit_address + 3)); 
		
	} else { // port range in CRH
		pin_bit_address -= 32;
		
		// set MODE bits HIGH, 11 = output, 50MHz
		GPIO->CRH |= (1u << pin_bit_address) | (1u << (pin_bit_address + 1)); 

		// set CNF bits LOW, 00 = push-pull (in output mode)
		GPIO->CRH &= ~(1u << (pin_bit_address + 2)) & ~(1u << (pin_bit_address + 3)); 
	}
	
	return;
}


void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number) {
	// enable a specified port as input (button press)
	
	// convert port_number to half-byte position
	// used for shifting bit set/clear masks
	int pin_bit_address = port_number << 2;
	
	// distinguish between CRL (ports 0-7) and CRH (ports 8-15)
	if (port_number < 8) {
		
		// clear MODE bits to LOW (00 = input mode)
		GPIO->CRL &= ~(0x11u << (pin_bit_address));
		
		// set CNF bits to 10 = pull-up/down (in input mode)
		// set upper CNF bit to HIGH
		GPIO->CRL |= (0x1u << (pin_bit_address + 3));
		
		// clear lower CNF bit to LOW
		GPIO->CRL &= ~(0x1u << (pin_bit_address + 2));	
			
	} else { // port range in CRH
		pin_bit_address -= 32; 
		
		// clear MODE bits to LOW (00 = input mode)
		GPIO->CRH &= ~(0x11u << (pin_bit_address));
		
		// set CNF bits to 10 = pull-up/down (in input mode)
		GPIO->CRH |= (0x1u << (pin_bit_address + 3));
		GPIO->CRH &= ~(0x1u << (pin_bit_address + 2));
	}		
	return; 
}


int main(void) {
	
	// enable clock for ports A, B, C, D
	RCC->APB2ENR |= (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5); // set bits 2, 3, 4, 5 HIGH

	
	// set port A pins 0-3 and 5 to output
	enable_GPIO_output(GPIOA, 0);		// board LEDs
	enable_GPIO_output(GPIOA, 1);
	enable_GPIO_output(GPIOA, 2);
	enable_GPIO_output(GPIOA, 3);
	enable_GPIO_output(GPIOA, 5); 	// green LED on STM32F103RB board

	// set port C13 as input
	enable_GPIO_input(GPIOC, 13); 	// blue button on STM32F103RB board
	
	// set port D5, 10, 14, and 15 as input 
	enable_GPIO_input(GPIOD, 5); 	// black button
	enable_GPIO_input(GPIOD, 10); // red
	enable_GPIO_input(GPIOD, 14); // blue
	enable_GPIO_input(GPIOD, 15); // green
	
	while (1) {
	
		write_input_to_output(GPIOC, GPIOA, 13, 5);
		write_input_to_output(GPIOD, GPIOA, 5, 0);
		write_input_to_output(GPIOD, GPIOA, 10, 1);
		write_input_to_output(GPIOD, GPIOA, 15, 2);
		write_input_to_output(GPIOD, GPIOA, 14, 3);
		//button_control_LED_step();
		
	}
	
	return 0;
}
