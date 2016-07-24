/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The planner module, which is responsible for consuming
 * pin/key events, and for deciding where the elevator
 * should go next
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "global.h"
#include "planner.h"
#include "assert.h"
#include "pin_listener.h" //for testing

#define FLOOR_1_HIGHT 0
#define FLOOR_2_HIGHT 400
#define FLOOR_3_HIGHT 800


/* to determine the direction of elevator */
#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)

void floorToVisit(void);
void arrivedAtFloor(void);

/* global variable for floor flags */
int floorFlags[4] = {1, 1, 1, 1};

void floorToVisit(void)
{
		if(floorFlags[0]){
			floorFlags[3] = 1;
			setCarTargetPosition(FLOOR_1_HIGHT);
			printf("Go to floor 1\n");
			setCarMotorStopped(0);
		}
		else if (floorFlags[1]){
			floorFlags[3] = 2;
			setCarTargetPosition(FLOOR_2_HIGHT);
			printf("Go to floor 2\n");
			setCarMotorStopped(0);
		}
		else if (floorFlags[2]){
			floorFlags[3] = 3;
			setCarTargetPosition(FLOOR_3_HIGHT);
			printf("Go to floor 3\n");
			setCarMotorStopped(0);
		}
		else
			printf("No floor to go\n");
			
		printf("Floor flags when start move: %i, %i, %i, %i\n", floorFlags[0], floorFlags[1], floorFlags[2], floorFlags[3]);
}

int getCurrentFloor(void)
{
	int position = getCarPosition();
	return (position + 1) / FLOOR_2_HIGHT + 1;
}

static void plannerTask(void *params) {
  
  PinEvent pvBuffer = UNASSIGNED;
  portBASE_TYPE xStatus;
  int curFloor = 0;
  portTickType xTaskLastWakeTime;
	
  floorFlags[0] = 0;
	floorFlags[1] = 0;
	floorFlags[2] = 0;
	floorFlags[3] = 1;
    
  /* Receive from the queue */
	xTaskLastWakeTime = xTaskGetTickCount();
	
  for (;;) {
	  xStatus = xQueueReceive(pinEventQueue, &pvBuffer, portMAX_DELAY);
	  
      if (xStatus == pdPASS) {  
				switch (pvBuffer) {
					case TO_FLOOR_1:
						printf("Button to floor 1 pressed\n");
						floorFlags[0] = 1;
						printf("Floor flags when button pressed: %i, %i, %i, %i\n", floorFlags[0], floorFlags[1], floorFlags[2], floorFlags[3]);
						break;
					
					case TO_FLOOR_2:
						printf("Button to floor 2 pressed\n");
						floorFlags[1] = 1;
						printf("Floor flags when button pressed: %i, %i, %i, %i\n", floorFlags[0], floorFlags[1], floorFlags[2], floorFlags[3]);
						if( getCarPosition() <= 0.5*FLOOR_2_HIGHT){
							printf("Next stop at floor 2\n");
							floorFlags[3] = 2;
							floorToVisit();
						}
						break;
					
					case TO_FLOOR_3:
						printf("Button to floor 3 pressed\n");
						floorFlags[2] = 1;
						printf("Floor flags when button pressed: %i, %i, %i, %i\n", floorFlags[0], floorFlags[1], floorFlags[2], floorFlags[3]);
						break;
					
					case ARRIVED_AT_FLOOR:
						curFloor = getCurrentFloor();
						if(floorFlags[3] == curFloor) { // stop motor only when arriving the floor in the plan
//							setCarMotorStopped(1);
							printf("Arrived At Floor, Current floor: %i\n", curFloor);
							floorFlags[floorFlags[3]-1] = 0; // clear the floor flag
							printf("Floor flags when clear flags: %i, %i, %i, %i\n", floorFlags[0], floorFlags[1], floorFlags[2], floorFlags[3]);
							printf("Wait 1 sec at the floor\n");
							vTaskDelay(1000 / portTICK_RATE_MS);
							setCarMotorStopped(1);
						}
						break;
					
					case STOP_PRESSED:
						printf("Stop button is pressed\n");
						setCarMotorStopped(1);
						break;
					
					case STOP_RELEASED:
						printf("Stop button is released\n");
						floorToVisit();
						break;
					
					case DOORS_CLOSED:
						printf("Door closed\n");
						floorToVisit();
						break;
					
					case DOORS_OPENING:
						printf("Door opening\n");
						xTaskLastWakeTime = xTaskGetTickCount();
						break;
					
					default:
						 printf("Default case, do nothing\n");	
				} // end switch
			} // end if pdPASS
			else
				printf("Nothing in the queue\n");
			
		} // end for loop
}

void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}
