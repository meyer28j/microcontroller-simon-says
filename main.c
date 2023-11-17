
#include "main.h"

// represents button input
// -1  = no input
// 0-3 = corresponding button was pressed
int input_global = -1;	

// regularly incremented as program runs
// to produce pseudo-random number for srand()
int seed_counter = 0;

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
	//uint32_t volatile internal_delay = delay_value;
	
	while (count < delay_value) {
		while (inner_count < delay_value) { //internal_delay) {
			inner_count++;
			//seed_counter++;
		}
		inner_count = 0;
		count++;
	}
}


void led_on(int led_number) {
	if (led_number < 0 || led_number > 3) {
		return;
	}
	led[led_number].GPIO->ODR |= (1u << led[led_number].pin);
	return;
}


void led_off(int led_number) {
	if (led_number < 0 || led_number > 3) {
		return;
	}
	led[led_number].GPIO->ODR &= ~(1u << led[led_number].pin);
	return;
}



void blink(int led_number, uint32_t duration) {
	// power on LED (0, 1, 2, or 3) for specified duration
	if (led_number < 0 || led_number > 3) { // illegal values
		return;
	}
		led_on(led_number);
	delay(duration);
	led_off(led_number);

}


void blink_multi(int led_number[], int array_size, uint32_t duration) {
	// pass in array of which LEDs you want to power
	// e.g. [1, 3] would turn on the 2nd and 4th LEDs
	// e.g. [0, 2, 3] would turn on LEDs 1, 3, and 4
	for (int i = 0; i < array_size; i++) {
		if (led_number[i] >= 0 && led_number[i] < 4) { // verify led_number is legal value
			led_on(led_number[i]);
		}
	}
	delay(duration);
	for (int i = 0; i < array_size; i++) {
		if (led_number[i] >= 0 && led_number[i] < 4) { // verify led_number is legal value
			led_off(led_number[i]);
		}
	}
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


int timer_button_interrupt(uint32_t volatile max_time) {
	// returns button value of user inputs value
	// else returns 0 if max_time elapsed
	// without input from user
	
	// similar function as delay() but
	// returns user input (or lack thereof)
	
	uint32_t volatile count = 0;
	uint32_t volatile inner_count = 0;
	
	// reset input_global, since it's used
	// as our exit condition
	input_global = -1;
	
	while (count < max_time) {
		while (inner_count < max_time) {
			inner_count++;
		}
		inner_count = 0;
		count++;
		detect_input_global(1);
		if (input_global != -1) { // placed on outer loop so not as expensive
			return input_global;
		}
	}
	return -1;
}


int timer_button_interrupt_with_seeding(uint32_t volatile max_time) {
	// special version of timer_button_interrupt()
	// that rapidly increments the random seed while
	// running to provide a pseudo-random number
	
	uint32_t volatile count = 0;
	uint32_t volatile inner_count = 0;
	
	// reset input_global, since it's used
	// as our exit condition
	input_global = -1;
	
	while (count < max_time) {
		while (inner_count < max_time) {
			inner_count++;
			seed_counter++;
			detect_input_global(1);
		if (input_global != -1) { // placed on inner loop to poll input as frequently as possible
			return input_global;
		}
		}
		inner_count = 0;
		count++;

	}
	return -1;
}



int* generate_light_sequence(int size) {
	int sequence[size];
	for (int i = 0; i < size; i++) {
		// collect numbers between 0-3 by shifting
		// bits 29 and 30 of rand() to positions 0 and 1
		// since we only need 2 bits to represent 4 numbers
		sequence[i] = (rand() >> 29) & 0x3;
	}
	int* array_pointer = sequence;
	return array_pointer;
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


void initialize_leds_buttons(void) {
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

	return;
}


void display_knight_rider(uint32_t blink_speed) {
	// function runs indefinitely until user presses a button
	int input = -1;
	while (input == -1) {
		for (int i = 0; i < 4; i++) { // flash lights right-to-left
			led_on(i);
			input = timer_button_interrupt_with_seeding(blink_speed);
			led_off(i);
			if (input != -1) { // stop on user input
				break;
			}
		}
		if (input != -1) { // don't start second loop if user input
				break;
			}
		for (int i = 2; i > 0; i--) { // flash lights left-to-right
			led_on(i);
			input = timer_button_interrupt_with_seeding(blink_speed);
			led_off(i);
			if (input != -1) { // stop on user input
				break;
			}
		}
	}
	return;
}


void display_binary(int number) {
	// convert number to binary
		int led_positions[4] = {-1, -1, -1, -1};
		int led_count = 0;
		
		if (number >= 8) {
			number -= 8;
			led_positions[led_count] = 3; // enable 8s position LED
			led_count++;
		}
		if (number >= 4) {
			number -= 4;
			led_positions[led_count] = 2; // enable 4s position LED
			led_count++;
		}
		if (number >= 2) {
			number -= 2;
			led_positions[led_count] = 1; // enable 2s position LED
			led_count++;
		}
		if (number >= 1) {
			number -= 1;
			led_positions[led_count] = 0; // enable 1s position LED
			led_count++;
		}
		for (int i = 0; i < 4; i++) {
			// turn on LEDs representing score
			led_on(led_positions[i]);
		}
		return;
}



int main(void) {
	
	initialize_ports();
	initialize_leds_buttons();
		
	int all_leds[4] = {0, 1, 2, 3};

	uint32_t blink_speed = 600;
	
	// runs indefinitely until user presses a button
	display_knight_rider(blink_speed  / 2);

	// seed random number generator
	srand((unsigned)seed_counter);

	int round;	
	int round_total = 5;
	
	// populate list of random numbers from 0-3
	// to represent the game sequence
	int* light_sequence = generate_light_sequence(round_total);
	
	// allocate memory for tracking user input
	int input_sequence[round_total];

	// boolean to check incorrect input for game-fail
	int winning = 1; 
	
	// length of time user has to push a button
	uint32_t input_window = 4000;
	
	
	// BEGIN GAME LOOP
	for (round = 1; round <= round_total && winning == 1; round++) {
		delay(2000); // breathing room at start of round
		
		// show player pattern they need to match
		// for this round
		for (int i = 0; i < round; i++) {
			blink(light_sequence[i], 2000);
			delay(1000);
		}
		
		// collect user input in order
		for (int i = 0; i < round; i++) {
			input_sequence[i] = timer_button_interrupt(input_window);		
			
			// flash light user entered for feedback
			blink(input_sequence[i], blink_speed);
			
			// wait out switch bounce
			delay(1500);
			
			// if the user enters any wrong (or no) input, it cancels the game
			// immediately without allowing for another input cycle
			if (input_sequence[i] != light_sequence[i]) { 
				winning = 0;
				break;
			} 
		}
		
		delay(1000);
		// end of round
		// clear any user input so they are 
		// required to enter it all again
		for (int i = 0; i < round; i++) {
			input_sequence[i] = -1;
		}
	}
		
	// lose sequence
	if (winning == 0) {
		for (int i = 0; i < 20; i++) {
			// flash one light
			blink(3, blink_speed);
			delay(blink_speed);
		}
		
		delay(1000);

		// display score
		round -= 2; // don't count last round, plus handle offset from starting at 1
		display_binary(round);
		return 1; // end program with final score displayed
	}
	
	// not losing and the game is over?
	// we found ourselves a winner!!!
	while (1) {
		blink_multi(all_leds, 4, blink_speed * 2);
		delay(blink_speed * 2);
	}
	
	return 0;
}
