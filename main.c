#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_usart.h"

#include "xprintf.h"


#define FLASH_0LAT_DELAY0LAT
//#define FLASH_0LAT_DELAY1LAT
//#define FLASH_1LAT_DELAY0LAT
//#define FLASH_1LAT_DELAY1LAT

/**
  * System Clock Configuration
  * The system Clock is configured as follow :
  *    System Clock source            = PLL (HSI/2)
  *    SYSCLK(Hz)                     = 48000000
  *    HCLK(Hz)                       = 48000000
  *    AHB Prescaler                  = 1
  *    APB1 Prescaler                 = 1
  *    HSI Frequency(Hz)              = 8000000
  *    PLLMUL                         = 12
  *    Flash Latency(WS)              = 1
  */
static void rcc_config() {
	/* Set FLASH latency */
#if defined(FLASH_0LAT_DELAY0LAT) || defined(FLASH_0LAT_DELAY1LAT)
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
#else
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
#endif

	/* Enable HSI and wait for activation*/
	LL_RCC_HSI_Enable();
	while (LL_RCC_HSI_IsReady() != 1);

	/* Main PLL configuration and activation */
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
	LL_RCC_PLL_MUL_12);

	LL_RCC_PLL_Enable();
	 while (LL_RCC_PLL_IsReady() != 1);

	/* Sysclk activation on the main PLL */
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

	/* Set APB1 prescaler */
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	/* Update CMSIS variable (which can be updated also
	 * through SystemCoreClockUpdate function) */
	SystemCoreClock = 48000000;
}

static void gpio_config(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
	return;
}

static void tim_config() {
	/*
	 * Configure output channel
	 */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_5, LL_GPIO_AF_2);

	/*
	 * Setup timer to output compare mode
	 */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_TIM_SetPrescaler(TIM2, 47);
	LL_TIM_SetAutoReload(TIM2, 2272);
	LL_TIM_OC_SetCompareCH1(TIM2, 1136);
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
	LL_TIM_OC_SetPolarity(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);
//	LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_TOGGLE);
	LL_TIM_OC_SetMode(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1);
	LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
//	LL_TIM_EnableIT_CC1(TIM2);
//	LL_TIM_EnableCounter(TIM2);
	/*
	 * Setup NVIC
	 */
//	NVIC_EnableIRQ(TIM2_IRQn);
//	NVIC_SetPriority(TIM2_IRQn, 1);
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	LL_TIM_SetPrescaler(TIM3, 479);
	LL_TIM_SetAutoReload(TIM3, 99999);
	LL_TIM_SetCounterMode(TIM3, LL_TIM_COUNTERMODE_UP);
	LL_TIM_EnableIT_UPDATE(TIM3);
	/*
	 * Setup NVIC
	 */
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_SetPriority(TIM3_IRQn, 0);
	
	return;
}

/*
 * Initialize USART module and associated pins
 */
static void usart_config(void) {
	/*
	 * Setting USART pins
	 */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	//USART1_TX
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_9, LL_GPIO_AF_1);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
	//USART1_RX
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_10, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_8_15(GPIOA, LL_GPIO_PIN_10, LL_GPIO_AF_1);
	LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_10, LL_GPIO_SPEED_FREQ_HIGH);
	/*
	 * USART Set clock source
	 */
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
	/*
	 * USART Setting
	 */
	LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);
	LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
	LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
	LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
	LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST);
	LL_USART_SetBaudRate(USART1, SystemCoreClock,
						LL_USART_OVERSAMPLING_16, 115200);
	LL_USART_EnableIT_IDLE(USART1);
	LL_USART_EnableIT_RXNE(USART1);
	/*
	 * USART turn on
	 */
	LL_USART_Enable(USART1);
	while (!(LL_USART_IsActiveFlag_TEACK(USART1) &&
			LL_USART_IsActiveFlag_REACK(USART1)));
	/*
	 * Turn on NVIC interrupt line
	 */
	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_EnableIRQ(USART1_IRQn);
	return;
}

void USART1_IRQHandler(void) {
	if (LL_USART_IsActiveFlag_RXNE(USART1)) {
//		LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
		char number = LL_USART_ReceiveData8(USART1);
		LL_USART_TransmitData8(USART1, number);
//		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_8);
		LL_TIM_DisableCounter(TIM2);
		switch (number) {
		case '1':
			LL_TIM_SetAutoReload(TIM2, 3821);
			LL_TIM_OC_SetCompareCH1(TIM2, 1911);
			break;	
		case '2':
			LL_TIM_SetAutoReload(TIM2, 3607);
			LL_TIM_OC_SetCompareCH1(TIM2, 1804);
			break;	
		case '3':
			LL_TIM_SetAutoReload(TIM2, 3404);
			LL_TIM_OC_SetCompareCH1(TIM2, 1702);
			break;	
		case '4':
			LL_TIM_SetAutoReload(TIM2, 3228);
			LL_TIM_OC_SetCompareCH1(TIM2, 1614);
			break;	
		case '5':
			LL_TIM_SetAutoReload(TIM2, 3061);
			LL_TIM_OC_SetCompareCH1(TIM2, 1531);
			break;	
		case '6':
			LL_TIM_SetAutoReload(TIM2, 2871);
			LL_TIM_OC_SetCompareCH1(TIM2, 1436);
			break;	
		case '7':
			LL_TIM_SetAutoReload(TIM2, 2706);
			LL_TIM_OC_SetCompareCH1(TIM2, 1353);
			break;	
		case '8':
			LL_TIM_SetAutoReload(TIM2, 2550);
			LL_TIM_OC_SetCompareCH1(TIM2, 1275);
			break;	
		case '9':
			LL_TIM_SetAutoReload(TIM2, 2407);
			LL_TIM_OC_SetCompareCH1(TIM2, 1204);
			break;	
		case '0':
			LL_TIM_SetAutoReload(TIM2, 2272);
			LL_TIM_OC_SetCompareCH1(TIM2, 1136);
			break;
		case '-':
			LL_TIM_SetAutoReload(TIM2, 2144);
			LL_TIM_OC_SetCompareCH1(TIM2, 1072);
			break;	
		case '=':
			LL_TIM_SetAutoReload(TIM2, 2024);
			LL_TIM_OC_SetCompareCH1(TIM2, 1012);
			break;
		
		}
		if ((number >= '0' && number <= '9') || (number == '-') || (number == '=')) {
//			LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_8);
//			LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_9);
			LL_TIM_DisableCounter(TIM3);
			LL_TIM_SetCounter(TIM2, 0);
			LL_TIM_SetCounter(TIM3, 0);
			LL_TIM_EnableCounter(TIM2);
			LL_TIM_EnableCounter(TIM3);
		}
	}
	
	if (LL_USART_IsActiveFlag_IDLE(USART1)) {
		LL_USART_ClearFlag_IDLE(USART1);
	}
}

void TIM3_IRQHandler() {
//	LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_8);
//	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_8);
	LL_TIM_DisableCounter(TIM2);
	LL_TIM_DisableCounter(TIM3);
	
	LL_TIM_ClearFlag_UPDATE(TIM3);
}

int main(void) {
	rcc_config();
	gpio_config();
	tim_config();
	usart_config();
	
	while (1);
	return 0;
}

