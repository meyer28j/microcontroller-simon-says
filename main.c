
#include "main.h"

// represents button input
// -1  = no input
// 0-3 = corresponding button was pressed
int input_global = -1;	

static volatile struct {
	GPIO_TypeDef* GPIO;
	int pin;
} led[4];

static volatile struct {
	GPIO_TypeDef* GPIO;
	int pin;
} button[4];


void delay(uint32_t volatile delay_value) {
	uint32_t volatile count = 0;
	uint32_t volatile inner_count = 0;
	uint32_t volatile internal_delay = delay_value;
	
	while (count < delay_value) {
		while (inner_count < internal_delay) {
			inner_count++;
		}
		inner_count = 0;
		count++;
	}
}

void blink(int led_number, uint32_t duration) {
	// power on LED (0, 1, 2, or 3) for specified duration
	if (led_number < 0 || led_number > 3) { // illegal values
		return;
	}
	
	led[led_number].GPIO->ODR |= (1u << led[led_number].pin);
	delay(duration);
	led[led_number].GPIO->ODR &= ~(1u << led[led_number].pin);

}


int detect_input(void) {
	// return values:
	// -1: no input
	//  0: black button was pressed
	//  1: red button
	//  2: green button
	//  3: blue button
	// This function assumes one button is pressed
	// at a time. If more than one button is pressed
	// at runtime, only the "lowest" button will be read
	
	uint32_t input_value;
	for (int i = 0; i < 4; i++) {
		// isolate input bit by shifting to zero position
		input_value = button[i].GPIO->IDR >> button[i].pin;
		input_value = ~input_value; // active low buttons
		input_value &= 1;
		
		if (input_value == 1) {
			return i;
		}
	}
	return -1;
}


void detect_input_global(int trigger) {
	// if trigger is set to non-zero, input_global
	// will only update on button presses and will
	// not update for no-input state
	
	uint32_t input_value;
	for (int i = 0; i < 4; i++) {
		// isolate input bit by shifting to zero position
		input_value = button[i].GPIO->IDR >> button[i].pin;
		input_value = ~input_value; // active low buttons
		input_value &= 1;
		
		if (input_value == 1) {
			input_global = i;
			return;
		}
	}
	if (trigger == 0) {
		input_global = -1;
	}
	return;
}


int timer_interruptable(uint32_t max_time) {
	// returns 1 if user inputs value
	// else returns 0 if max_time elapsed
	// without input from user
	return 1;
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


void initialize_ports(void) {
	// enable clock for ports A, B, C
	RCC->APB2ENR |= (1u << 2) | (1u << 3) | (1u << 4); // set bits 2, 3, 4, 5 HIGH

	// enable on-board button/LED control
	enable_GPIO_output(GPIOA, 5); 	// green LED on STM32F103RB board
	enable_GPIO_input(GPIOC, 13); 	// blue button on STM32F103RB board
	
	// enable output ports for board LEDs
	enable_GPIO_output(GPIOA, 0);
	enable_GPIO_output(GPIOA, 1);
	enable_GPIO_output(GPIOA, 4);
	enable_GPIO_output(GPIOB, 0);

	
	// enable input ports for board buttons
	enable_GPIO_input(GPIOB, 4); // black button
	enable_GPIO_input(GPIOB, 6); // red
	enable_GPIO_input(GPIOB, 8); // blue
	enable_GPIO_input(GPIOB, 9); // green
}


int main(void) {
	
	time_t seed_timer = time(NULL);
	initialize_ports();
	
	// statically bind each LED GPIO port and pin
	led[0].GPIO = GPIOA;
	led[0].pin = 0;
	led[1].GPIO = GPIOA;
	led[1].pin = 1;
	led[2].GPIO = GPIOA;
	led[2].pin = 4;
	led[3].GPIO = GPIOB;
	led[3].pin = 0;
	
	// statically bind each button and pin
	button[0].GPIO = GPIOB;
	button[0].pin = 4;
	button[1].GPIO = GPIOB;
	button[1].pin = 6;
	button[2].GPIO = GPIOB;
	button[2].pin = 8;
	button[3].GPIO = GPIOB;
	button[3].pin = 9;
	
	uint32_t blink_speed = 800;
	
	//int user_input = -1;
	while (input_global == -1) {
		blink(0, blink_speed);
		blink(1, blink_speed);
		blink(2, blink_speed);
		blink(3, blink_speed);
		blink(2, blink_speed);
		blink(1, blink_speed);
		detect_input_global(1);
	}
	
	// seed random number generator
	// TODO: convert to generic counter
	srand((uint32_t)(difftime(seed_timer, time(NULL))));
	
	int random = 0;
	// for a while, blink random lights
	for (int i = 0; i < 10; i++) {
		random = rand(); // collect upper 2 bits of RAND_MAX
		random >>= 29;
		random &= 0x3;
		blink(random, 3000);
	}
	
	while (1) {
	
		write_input_to_output(GPIOC, GPIOA, 13, 5); // on-board button/LED control
		
		write_input_to_output(GPIOB, GPIOA, 4, 0); // black button/LED
		write_input_to_output(GPIOB, GPIOA, 6, 1); // red button/LED
		write_input_to_output(GPIOB, GPIOA, 8, 4); // green button/LED
		write_input_to_output(GPIOB, GPIOB, 9, 0); // blue button/LED
		
	}
	
	return 0;
}
