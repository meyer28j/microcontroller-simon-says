
#ifndef STM32F103RB_H
#define STM32F103RB_H
#endif
#include <stdint.h>

// map for device registers
typedef struct
{
	volatile uint32_t CR;
	volatile uint32_t CFGR;
	volatile uint32_t CIR;
	volatile uint32_t APB2RSTR;
	volatile uint32_t APB1RSTR;
	volatile uint32_t AHBENR;
	volatile uint32_t APB2ENR;
	volatile uint32_t APB1ENR;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
	
	uint32_t RESERVED;
	volatile uint32_t CFGR2;
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *) 0x40021000)


// map for GPIO registers
typedef struct
{
	volatile uint32_t CRL;
	volatile uint32_t CRH;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSR;
	volatile uint32_t BRR;
	volatile uint32_t LCK;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *) 0x40010800)
#define GPIOB ((GPIO_TypeDef *) 0x40010C00)
