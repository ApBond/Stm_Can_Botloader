#include "rcc.h"

RCC_STATUS_t RccClockInit(void)
{
	//Enable HSE
	//Setting PLL
	//Enable PLL
	//Setting count wait cycles of FLASH
	//Setting AHB1,AHB2 prescaler
	//Switch to PLL	
	uint16_t timeDelay;
	RCC->APB1ENR|=RCC_APB1ENR_PWREN;
	RCC->AHBENR|=RCC_AHBENR_GPIOFEN;
    RCC->AHBENR|=RCC_AHBENR_GPIOCEN;
	RCC->AHBENR|=RCC_AHBENR_GPIOAEN;
    RCC->AHBENR|=RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR|=RCC_APB2ENR_SYSCFGEN;
	RCC->CR|=RCC_CR_HSEBYP;//HSE Bypass
	RCC->CR|=RCC_CR_HSEON;//Enable HSE
	for(timeDelay=0;;timeDelay++)
	{
		if(RCC->CR&RCC_CR_HSERDY) break;
		if(timeDelay>0x5000)
		{
			RCC->CR&=~RCC_CR_HSEON;
			return HSE_NOT_ENABLE;
		}
	}	
	RCC->CFGR=0;
	RCC->CFGR|=RCC_CFGR_PLLMUL9;//PLLx9
    RCC->CFGR|=RCC_CFGR_PLLSRC;
	RCC->CR|=RCC_CR_PLLON;//Enable PLL
	for(timeDelay=0;;timeDelay++)
	{
		if(RCC->CR&RCC_CR_PLLRDY) break;
		if(timeDelay>0x1000)
		{
			RCC->CR&=~RCC_CR_HSEON;
			RCC->CR&=~RCC_CR_PLLON;
			return PLL_NOT_ENABLE;
		}
	}
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	RCC->CFGR|=RCC_CFGR_PPRE1_DIV2;//APB1 prescaler=2
	RCC->CFGR|=RCC_CFGR_SW_PLL;//Switch to PLL
	while((RCC->CFGR&RCC_CFGR_SWS)!=(0x02<<2)){}
	//RCC->CR&=~RCC_CR_HSION;//Disable HSI
	
	return READY;
}

uint32_t delayInit()
{
	return SysTick_Config(SYS_CORE_FREQUENSY/1000);
}

void delay_ms(volatile uint32_t nTime)
{
	TimingDelay=nTime;
 	while(TimingDelay!=0){}
}

void delay_us(volatile uint32_t nTime)
{
	TimingDelay=nTime;
 	while(TimingDelay!=0){}
}

void timingDelayDecrement(void)
{
	if(TimingDelay!=0) TimingDelay--;
}

void SysTick_Handler(void)
{
	timingDelayDecrement();
}


