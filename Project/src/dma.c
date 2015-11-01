#include "stm32f10x.h"
#include <stdio.h>
#include <usart.h>


void uart1_dma_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel4);

	/*-------------- Reset DMA init structure parameters values ------------------*/
	  /* Initialize the DMA_PeripheralBaseAddr member */
	  DMA_InitStructure.DMA_PeripheralBaseAddr = 0;
	  /* Initialize the DMA_MemoryBaseAddr member */
	  DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	
	  /* Initialize the DMA_DIR member */
	  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	  /* Initialize the DMA_BufferSize member */
	  DMA_InitStructure.DMA_BufferSize = 0;
	  /* Initialize the DMA_PeripheralInc member */
	  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	  /* Initialize the DMA_MemoryInc member */
	  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	  /* Initialize the DMA_PeripheralDataSize member */
	  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	  /* Initialize the DMA_MemoryDataSize member */
	  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	  /* Initialize the DMA_Mode member */
	  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	  /* Initialize the DMA_Priority member */
	  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	  /* Initialize the DMA_M2M member */
	  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	 DMA_Init(DMA1_Channel4, &DMA_InitStructure);
}

