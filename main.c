
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


void enable_GPIO_output(GPIO_TypeDef* GPIO, int port_number) {
	// enable a specified port as output, 50MHz push-pull
	
	// distinguish between CRL (ports 0-7) and CRH (ports 8-15)
	if (port_number < 8) {
		
		// set MODE bits HIGH, 11 = output, 50MHz
		GPIO->CRL |= (1u << (port_number << 2)) | (1u << ((port_number << 2) + 1)); 

		// set CNF bits LOW, 00 = push-pull (in output mode)
		GPIO->CRL &= ~(1u << ((port_number << 2) + 2)) & ~(1u << ((port_number << 2) + 3)); 
		
	} else { // port range in CRH
		port_number -= 8;
		// set MODE bits HIGH, 11 = output, 50MHz
		GPIO->CRH |= (1u << (port_number << 2)) | (1u << ((port_number << 2) + 1)); 

		// set CNF bits LOW, 00 = push-pull (in output mode)
		GPIO->CRH &= ~(1u << ((port_number << 2) + 2)) & ~(1u << ((port_number << 2) + 3)); 
	}
	
	return;
}


void enable_GPIO_input(GPIO_TypeDef* GPIO, int port_number) {
	// enable a specified port as input (button press)
	
	// distinguish between CRL (ports 0-7) and CRH (ports 8-15)
	if (port_number < 8) {
		
		// clear MODE bits to LOW (00 = input mode)
		GPIO->CRL &= ~(0x11u << ((port_number << 2) + 1));
		
		// set CNF bits to 10 = pull-up/down (in input mode)
		// set upper CNF bit to HIGH
		GPIO->CRL |= (0x1u << ((port_number << 2) + 3));
		
		// clear lower CNF bit to LOW
		GPIO->CRL &= ~(0x1u << ((port_number << 2) + 2));	
			
	} else { // port range in CRH
		port_number -= 8;
		// clear MODE bits to LOW (00 = input mode)
		GPIO->CRH &= ~(0x11u << ((port_number << 2) + 1));
		
		// set CNF bits to 10 = pull-up/down (in input mode)
		// set upper CNF bit to HIGH
		GPIO->CRH |= (0x1u << ((port_number << 2) + 3));
		
		// clear lower CNF bit to LOW
		GPIO->CRH &= ~(0x1u << ((port_number << 2) + 2));
	}		
	return; 
}


int main(void) {
	
	// enable clock for ports A, B, C, D
	RCC->APB2ENR |= (1u << 2) | (1u << 3) | (1u << 4) | (1u << 5); // set bits 2, 3, 4, 5 HIGH
	
	
	
	// enable control of port A5, green LED on STM32F103RB board
	// set port A MODE5 bits to 11 = output, 50MHz
	//GPIOA->CRL |= (1u << 20) | (1u << 21); // set bits 20 and 21 HIGH
	
	// clear port A CNF5 bits to 00 = push-pull (in output mode)
	//GPIOA->CRL &= ~(1u << 22) & ~(1u << 23); // clear bits 22 and 23 to LOW
	enable_GPIO_output(GPIOA, 5);
	
	
	// enable 
	// set to output mode
	// set port A MODE0 bits to 11 = output, 50MHz
	//GPIOA->CRL |= (1u) | (1u << 1); // set MODE0 bits 0 and 1 HIGH
	
	// clear port A CNF0 bits to 00 = push-pull (in output mode)
	//GPIOA->CRL &= ~(1u << 2) & ~(1u << 3); // clear bits 2 and 3 to LOW
	enable_GPIO_output(GPIOA, 0);
	enable_GPIO_output(GPIOA, 1);
	enable_GPIO_output(GPIOA, 2);
	enable_GPIO_output(GPIOA, 3);
	
		
		
	// enable port D5 as input (black button)
	// set port D MODE5 to 00 = input
	//GPIOD->CRL &= ~(0x11u << 21); // clear bits 20 and 21 to LOW (input mode)
	
	// set port D CNF5 to 10 = pull-up/pull-down (in input mode)
	//GPIOD->CRL |= (0x1u << 23); // set bit 23 to HIGH
	//GPIOD->CRL &= ~(0x1u << 22); // clear bit 22 to LOW
	enable_GPIO_input(GPIOD, 5);
	enable_GPIO_input(GPIOD, 10);
	enable_GPIO_input(GPIOD, 14);
	enable_GPIO_input(GPIOD, 15);
	
	
	// enable DAC (digital to analog converter) to enable
	// output to breadboard pins
	// DAC boundary address: 0x4000 7800
	
	// page 256 of STM32F103RB manual:
	// Each DAC channel can be powered on by setting its corresponding ENx bit in the DAC_CR
	// register. The DAC channel is then enabled after a startup time tWAKEUP
	
	
	// DAC_CR, offset 0x0
	// enable EN1 (bit 0) to enable power on one rail
	//DAC->CR |= (1u);
	
	// enable EN2 (bit 16) to enable other rail
	//DAC->CR |= (1u << 16);
	
	
	

	
	
	
	// enable user blue button control to toggle LED
	while (1) { // repeat indefinitely
	
		button_control_LED_step();
		
	}
	
	return 0;
}
