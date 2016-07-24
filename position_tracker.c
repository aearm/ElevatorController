/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Class for keeping track of the car position.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "position_tracker.h"
#include "assert.h"

static void positionTrackerTask(void *params) {
    PositionTracker * tracker= (PositionTracker *)params;
	u8 prevDirection = 0;
	u8 curDirection = 0;
    portTickType xLastWakeTime;
  
    xLastWakeTime = xTaskGetTickCount();
	
   for (;;) {
		 curDirection = GPIO_ReadInputDataBit(tracker->gpio, tracker->pin);
		 
		 if (curDirection > prevDirection) {
		 
         if(xSemaphoreTake(tracker->lock, portMAX_DELAY))
		     {
					 if (tracker->direction == Up) {
						  ++tracker->position;
					 }
						
					 
					 else if (tracker->direction == Down) {
						 --tracker->position;
					 }
						  
					 
					 else if (tracker->position == Unknown) {
						 tracker->position = tracker->position;
//						 printf("Nothing\n");
					 }
						 					 
					 xSemaphoreGive(tracker->lock);
		      }
		  		  
		 else printf("Semaphore take failed\n");
				    
	     }
		
		prevDirection = curDirection;
		
	    vTaskDelayUntil(&xLastWakeTime, tracker->pollingPeriod); 
  }
}
void setupPositionTracker(PositionTracker *tracker,
                          GPIO_TypeDef * gpio, u16 pin,
						  portTickType pollingPeriod,
						  unsigned portBASE_TYPE uxPriority) {
  portBASE_TYPE res;

  tracker->position = 0;
  tracker->lock = xSemaphoreCreateMutex();
  assert(tracker->lock != NULL);
  tracker->direction = Unknown;
  tracker->gpio = gpio;
  tracker->pin = pin;
  tracker->pollingPeriod = pollingPeriod;
	
  res = xTaskCreate(positionTrackerTask, "position tracker",
                    80, (void*)tracker, uxPriority, NULL);
  assert(res == pdTRUE);
}

void setDirection(PositionTracker *tracker, Direction dir) {

	xSemaphoreTake(tracker->lock, portMAX_DELAY);
	tracker->direction = dir;
    xSemaphoreGive(tracker->lock);
}

s32 getPosition(PositionTracker *tracker) {
  //vs32 pos;
  
  //xSemaphoreTake(tracker->lock, portMAX_DELAY);
  //pos = tracker->position;
  //xSemaphoreGive(tracker->lock);
  
  return tracker->position;
}