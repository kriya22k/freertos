/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file    FreeRTOS/FreeRTOS_ThreadCreation/Src/main.c
* @author  MCD Application Team
* @brief   Main program body
******************************************************************************
* @attention
*
* Copyright (c) 2019 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "FreeRTOS.h"
#include "stm32l5xx_hal_uart.h"
#include "stm32l5xx_hal_uart_ex.h"
#include "queue.h"



/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"


//#define RX_BUFFER_SIZE 100
#define QUEUE_LENGTH 100
#define QUEUE_USING_API

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


/* USER CODE BEGIN PV */

__IO uint32_t OsStatus = 0;
UART_HandleTypeDef huart1;
HAL_StatusTypeDef error;
extern QueueHandle_t xPointerQueue;
struct node *head = NULL;

static uint8_t help_msg[] =
	"\r\n\n=========================================================\r\n"
	"#  CLI Console:\r\n"
	"=========================================================\r\n"
	"A - Add data\r\n"
	"D - Delete data\r\n"
	"S - Search particular data\r\n"
	"P - Print all the data\r\n"
	"F - Store data\r\n"
	"E - Exit\r\n"
	"NOTE: Configure UART Terminal Transmit NewLine as <CR+LF>\r\n"
	"=========================================================\r\n"
	"User INPUT > ";


int length;
int i=0;
uint8_t msg[] = "Hi, Welcome to UART demo!!\r\n";
uint8_t invalid[]="\nWrong input!!!!\r\n";
uint8_t exit_msg[]="\nExited\n";
TaskHandle_t task1_handle;


void(*operations[])(struct node **)= {
		add_data,		//adding user data to database
		delete_data,	//deleting user data from database
		search_data,	//searching user data from database
		print_data,		//printing user data from database
		store_data		//adding database data to flash memory
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_ICACHE_Init(void);
static void MX_USART1_UART_Init(void);
static void EXTI13_IRQHandler_Config(void);
/* USER CODE BEGIN PFP */
static void vSenderTask( void *pvParameters );

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


/**
* @brief  The application entry point.
* @retval int
*/
int main(void)
{
	/* USER CODE BEGIN 1 */


	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */

	MX_ICACHE_Init();

	/* USER CODE BEGIN 2 */

	/* USER CODE END 2 */
	osKernelInitialize();
	EXTI13_IRQHandler_Config();
	MX_USART1_UART_Init();
	USART1->CR1 |= USART_CR1_RXNEIE;
	flash_intialization();
	backup(&head);
	HAL_UART_Transmit(&huart1, msg, sizeof(msg), 1000);
	BaseType_t status;
	xPointerQueue = xQueueCreate( QUEUE_LENGTH, sizeof( char * ) );
	if( xPointerQueue != NULL )
	{
	  status = xTaskCreate(vSenderTask, "Task1", 1500, NULL, 0, &task1_handle);
	  configASSERT(status == pdPASS);
	  uart_transmit(help_msg, sizeof(help_msg));
	  /* Start scheduler */
	  vTaskStartScheduler();
	}
	else
	{
	/* The queue could not be created. */
	  printf("Queue not created\n");
	}

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
	/* USER CODE END WHILE */
	  /* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}



/**
* @brief System Clock Configuration
* @retval None
*/
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	*/
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE0) != HAL_OK)
	{
	  Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	* in the RCC_OscInitTypeDef structure.
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 55;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	  Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
		  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
	  Error_Handler();
	}
}


/**
* @brief ICACHE Initialization Function
* @param None
* @retval None
*/
static void MX_ICACHE_Init(void)
{
	/* USER CODE BEGIN ICACHE_Init 0 */

	/* USER CODE END ICACHE_Init 0 */

	/* USER CODE BEGIN ICACHE_Init 1 */

	/* USER CODE END ICACHE_Init 1 */

	/** Enable instruction cache in 1-way (direct mapped cache)
	*/
	if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_ICACHE_Enable() != HAL_OK)
	{
	  Error_Handler();
	}
	/* USER CODE BEGIN ICACHE_Init 2 */

	/* USER CODE END ICACHE_Init 2 */

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */


/**
* @brief interrupt and uart GPIO Initialization Function
* @param None
* @retval None
*/
static void EXTI13_IRQHandler_Config(void)
{
	GPIO_InitTypeDef   GPIO_InitStructure;
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
	  Error_Handler();
	}
	/* Enable GPIOC clock */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* Configure PC.13 pin as input floating */
	GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = BUTTON_USER_PIN;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure PA.9 pin as UART TX */
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_9;
	GPIO_InitStructure.Alternate =GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PA.10 pin as UART RX */
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Pin = GPIO_PIN_10;
	GPIO_InitStructure.Alternate =GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Enable and set usart1 Interrupt */
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
* @brief UART Initialization Function
* @param None
* @retval None
*/

static void MX_USART1_UART_Init(void)
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
	Error_Handler();
	}
}




/**
* @brief  USART1 IRQ Handler.
* This function serves as the interrupt handler for USART1. It is responsible
* for handling incoming data in the USART1 receive buffer and sending it to
* a FreeRTOS queue for processing in other task.
* @note   This function handles the interrupts generated by USART1 peripheral.
*         It reads the received data from the USART1 receive data register (RDR)
* @retval None
*/

void USART1_IRQHandler(void) // Change IRQ handler name as per your UART peripheral
{

	#ifdef QUEUE_USING_API
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if (USART1->ISR & USART_ISR_RXNE)
	{
	xQueueSendFromISR(xPointerQueue, (uint8_t*)&USART1->RDR, &xHigherPriorityTaskWoken);
	}

	if (xHigherPriorityTaskWoken)
	{
	// Actual macro used here is port specific.
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
#endif


/**
* @brief  Sender Task. Performing different operations of UART CLI
* @para pvParameters A pointer to task parameters (not used in this task).
* @retval None
*/
void vSenderTask( void *pvParamter )
{
	if(uxQueueMessagesWaiting( xPointerQueue ) == 100)
	{
	uart_transmit("Queue is full!\r\n",18);
	}

	while(1)
	{
	uint8_t data = 0;
	if (xQueueReceive(xPointerQueue, &data, 0) == pdTRUE)
	{
		switch(data)
		{
		case 'A':
		case 'A'+32:  operations[0](&head);//add_data(&head);
					  break;

		case 'D':
		case 'D'+32:  operations[1](&head);
					  break;

		case 'S':
		case 'S'+32:  operations[2](&head);
					  break;

		case 'P':
		case 'P'+32:  operations[3](&head);
					  break;

		case 'F':
		case 'F'+32:  operations[4](&head);
					  break;

		case 'E':
		case 'E'+32:  uart_transmit(exit_msg, sizeof(exit_msg));
					  goto exit;

		default:   uart_transmit(invalid, sizeof(invalid));
		}
	uart_transmit(help_msg, sizeof(help_msg));
	}
	}
	exit: ;
}



/*
/**
* @brief  Period elapsed callback in non blocking mode
* @note   This function is called  when TIM6 interrupt took place, inside
* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
* a global variable "uwTick" used as application time base.
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//USER CODE BEGIN Callback 0

	//  USER CODE END Callback 0
	if (htim->Instance == TIM6) {
	HAL_IncTick();
	}
	//  USER CODE BEGIN Callback 1

	// USER CODE END Callback 1
}

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
	//USER CODE BEGIN Error_Handler_Debug
	//User can add his own implementation to report the HAL error return state
	while(1)
	{
	}
	// USER CODE END Error_Handler_Debug
}

#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{}
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
