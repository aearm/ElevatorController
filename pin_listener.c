/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Functions listening for changes of specified pins
 */

#include "FreeRTOS.h"
#include "task.h"

#include "pin_listener.h"
#include "assert.h"

/* counter for GPIO pins, for press and release events */
int counter_GPIO_Pin = 0;


static void pollPin(PinListener *listener,
                    xQueueHandle pinEventQueue) {

									
	u8 status;
	
	//read current pin status 
	status = GPIO_ReadInputDataBit(listener->gpio, listener->pin);
	
	// button has been pressed
	if (status != listener->status) {
		++counter_GPIO_Pin;
		
		// de-bouncing for 2 times the polling period - 20ms
		if (counter_GPIO_Pin >= 2) {
			// send rising edge event
			if (status == 1) {
				xQueueSend(pinEventQueue, &(listener->risingEvent), portMAX_DELAY);
//				printf("Rising edge\n");
			}
			// send falling edge event to queue
			else {
				xQueueSend(pinEventQueue, &(listener->fallingEvent), portMAX_DELAY);
//				printf("Falling edge\n");
			}
			//new status, reset counter 
			listener->status = status;
			counter_GPIO_Pin = 0;
		}
	}

}

static void pollPinsTask(void *params) {
  PinListenerSet listeners = *((PinListenerSet*)params);
  portTickType xLastWakeTime;
  int i;
  
  xLastWakeTime = xTaskGetTickCount(); 
  
  for (;;) {
    for (i = 0; i < listeners.num; ++i)
	  pollPin(listeners.listeners + i, listeners.pinEventQueue);
    
	vTaskDelayUntil(&xLastWakeTime, listeners.pollingPeriod);
  }
}

void setupPinListeners(PinListenerSet *listenerSet) {
  portBASE_TYPE res;

  res = xTaskCreate(pollPinsTask, "pin polling",
                    100, (void*)listenerSet,
					listenerSet->uxPriority, NULL);
  assert(res == pdTRUE);
}