/**************************************************************************//**
 *
 * @file        IOUtilsCircularBufferRx.c
 * @brief       Part of FreeRTOS+IO
 * @author      Real Time Engineers Ltd.
 * @version     1.0.0
 * @date        25 July. 2012
 *
 * Copyright (C) 2012 Real Time Engineers ltd.
 * All rights reserved.
 *
******************************************************************************/

/* Standard includes. */
#include "string.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "FreeRTOS_Task.h"
#include "FreeRTOS_Semaphore.h"

/* Device specific library includes. */
#include "FreeRTOS_DriverInterface.h"
#include "FreeRTOS_IOUtilsCommon.h"

/*-----------------------------------------------------------*/

portBASE_TYPE xIOUtilsConfigureCircularBufferRx( Peripheral_Control_t * const pxPeripheralControl, const portBASE_TYPE xBufferSize )
{
portBASE_TYPE xReturn = pdFAIL;
Circular_Buffer_Rx_State_t *pxCircularBufferState;

	/* A peripheral is going to use a Circular_Buffer_Rx_State_t structure to
	control reception. */
	vIOUtilsCreateTransferControlStructure( &( pxPeripheralControl->pxRxControl ) );
	configASSERT( pxPeripheralControl->pxRxControl );

	if( pxPeripheralControl->pxRxControl != NULL )
	{
		/* Create the necessary structure. */
		pxCircularBufferState = pvPortMalloc( sizeof( Circular_Buffer_Rx_State_t ) );

		if( pxCircularBufferState != NULL )
		{
			pxCircularBufferState->xNewDataSemaphore = NULL;

			/* The structure just created contains a Semaphore handle.  Create
			the binary type semaphore too. */
			vSemaphoreCreateBinary( pxCircularBufferState->xNewDataSemaphore );

			if( pxCircularBufferState->xNewDataSemaphore != NULL )
			{
				/* The semaphore was created correctly.  Fill in the private
				data structure. */

				/* First ensure the semaphore starts in the desired state. */
				xSemaphoreTake( pxCircularBufferState->xNewDataSemaphore, 0U );

				pxCircularBufferState->pucBufferStart = pvPortMalloc( xBufferSize );

				if( pxCircularBufferState->pucBufferStart != NULL )
				{
					pxCircularBufferState->usBufferLength = ( uint16_t ) xBufferSize;
					pxCircularBufferState->usErrorState = 0U;
					pxCircularBufferState->usNextReadIndex = 0U;
					pxCircularBufferState->usCharCount = 0U;
					pxCircularBufferState->usNextWriteIndex = 0U;
					pxPeripheralControl->pxRxControl->pvTransferState = ( void * ) pxCircularBufferState;
					pxPeripheralControl->pxRxControl->ucType = ioctlUSE_CIRCULAR_BUFFER_RX;
					pxCircularBufferState->xBlockTime = portMAX_DELAY;
					xReturn = pdPASS;
				}
				else
				{
					/* The buffer could not be allocated, so everything
					allocated thus far will have to be freed again and an error
					returned.  First free the semaphore. */
					vSemaphoreDelete( pxCircularBufferState->xNewDataSemaphore );
					pxCircularBufferState->xNewDataSemaphore = NULL;
				}
			}

			if( pxCircularBufferState->xNewDataSemaphore == NULL )
			{
				/* The semaphore was not created successfully, or the buffer
				could not be allocated so the semaphore has been deleted.  Free
				the	Circular_Buffer_Rx_State_t structure and just return an error. */
				vPortFree( pxCircularBufferState );
				pxCircularBufferState = NULL;
			}
		}

		if( pxCircularBufferState == NULL )
		{
			/* The Rx structure, or a member it contains,  could not be created,
			so the Rx control structure (which should point to it) should also
			be deleted. */
			vPortFree( pxPeripheralControl->pxRxControl );
			pxPeripheralControl->pxRxControl = NULL;
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

void vIOUtilsClearRxCircularBuffer( Peripheral_Control_t * const pxPeripheralControl )
{
Circular_Buffer_Rx_State_t *pxCircularBufferState = prvCIRCULAR_BUFFER_STATE( pxPeripheralControl );

	pxCircularBufferState->usErrorState = 0U;
	pxCircularBufferState->usNextReadIndex = 0U;
	pxCircularBufferState->usCharCount = 0U;
	pxCircularBufferState->usNextWriteIndex = 0U;
	xSemaphoreTake( pxCircularBufferState->xNewDataSemaphore, 0U );
}
/*-----------------------------------------------------------*/

portBASE_TYPE xIOUtilsReadCharsFromCircularBuffer( Circular_Buffer_Rx_State_t * const pxCircularBuffer, uint8_t * const pucBuffer, const size_t xBytesToRead )
{
size_t xBytesAvailable, xBytesReadSoFar = 0U;
portBASE_TYPE xAttempts;
const portBASE_TYPE xMaxAttempts = 2;

	/* Two attempts at reading the required number of bytes are made in	case
	the first read just reads up to the end of the circular buffer before
	wrapping back round to the start - in which case the second	attempt will
	try and read the remaining bytes from the start. */
	for( xAttempts = 0; xAttempts < xMaxAttempts; xAttempts++ )
	{
		if( pxCircularBuffer->usCharCount == 0U )
		{
			/* There are no more characters in the buffer, so there is no point
			continuing. */
			break;
		}

		if( pxCircularBuffer->usNextReadIndex <= pxCircularBuffer->usNextWriteIndex )
		{
			/* How many bytes are available between the read and write
			indexes? */
			xBytesAvailable = ( pxCircularBuffer->usNextWriteIndex - pxCircularBuffer->usNextReadIndex );
		}
		else
		{
			/* How many bytes are available up to the end of the buffer? */
			xBytesAvailable = ( pxCircularBuffer->usBufferLength - pxCircularBuffer->usNextReadIndex );
		}

		/* Cap the bytes to read to a maximum of those asked for. */
		if( xBytesAvailable > ( xBytesToRead - xBytesReadSoFar ) )
		{
			xBytesAvailable = ( xBytesToRead - xBytesReadSoFar );
		}

		configASSERT( xBytesAvailable );

		/* Copy the bytes. */
		memcpy( &( pucBuffer[ xBytesReadSoFar ] ), &( pxCircularBuffer->pucBufferStart[ pxCircularBuffer->usNextReadIndex ] ), xBytesAvailable );

		/* Remember how many bytes have been read for the next iteration of
		this loop. */
		xBytesReadSoFar += xBytesAvailable;

		/* Characters have been removed from the buffer. */
		pxCircularBuffer->usCharCount -= xBytesAvailable;
		pxCircularBuffer->usNextReadIndex += xBytesAvailable;

		configASSERT( pxCircularBuffer->usNextReadIndex <= pxCircularBuffer->usBufferLength );

		if( pxCircularBuffer->usNextReadIndex == pxCircularBuffer->usBufferLength )
		{
			pxCircularBuffer->usNextReadIndex = 0U;
		}

		if( xBytesToRead == xBytesReadSoFar )
		{
			/* No more bytes are needed. */
			break;
		}
	}

	return xBytesReadSoFar;
}
/*-----------------------------------------------------------*/

void vIOUtilsSetCircularBufferTimeout( Peripheral_Control_t *pxPeripheralControl, portTickType xMaxWaitTime )
{
Transfer_Control_t *pxTransferControlState = pxPeripheralControl->pxRxControl;
Circular_Buffer_Rx_State_t *pxCircularBufferState;

	pxCircularBufferState = ( Circular_Buffer_Rx_State_t * ) ( pxTransferControlState->pvTransferState );
	pxCircularBufferState->xBlockTime = xMaxWaitTime;
}











