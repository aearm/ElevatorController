/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observes the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include <stdlib.h>

#include "global.h"
#include "assert.h"
#include "motor.h"

#define POLL_TIME (10 / portTICK_RATE_MS)

#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)
#define MOTOR_UPWARD_SPEED ((float)(TIM3->CCR1) / 200.0)
#define MOTOR_DOWNWARD_SPEED ((float)(TIM3->CCR2) / 200.0)


#define BUTTON1_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_0)
#define BUTTON2_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1)
#define BUTTON3_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2)

#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define AT_FLOOR      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

#define FLOOR_1_HIGHT 0
#define FLOOR_2_HIGHT 400
#define FLOOR_3_HIGHT 800

static portTickType xLastWakeTime;

static void check(u8 assertion, char *name) {
  if (!assertion) {
    printf("SAFETY REQUIREMENT %s VIOLATED: STOPPING ELEVATOR\n", name);
    for (;;) {
	  setCarMotorStopped(1);
  	  vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
    }
  }
}

static void safetyTask(void *params) {
	s32 timeSinceButton1Pressed = -1;
	s32 timeSinceButton2Pressed = -1;
	s32 timeSinceButton3Pressed = -1;
	
  s16 timeSinceStopPressed = -1;
	s16 timeSinceElevatorAtFloor = -1;
	s32 carPosition = 0;
	/*
	s16 stopped = 0;
	s16 doorsWereOpen = 0;
	*/
	s16 lastUpwardSpeed = 0;
	s16 lastDownwardSpeed = 0;
	
	int last_StopPressed = 0;
	
  xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    // Environment assumption 1: the doors can only be opened if
	//                           the elevator is at a floor and
    //                           the motor is not active

	check((AT_FLOOR && MOTOR_STOPPED) || DOORS_CLOSED,
	      "env1");

	// fill in environment assumption 2
	check(MOTOR_UPWARD_SPEED <= 50.0 && MOTOR_DOWNWARD_SPEED <= 50.0, "env2");

	// fill in environment assumption 3
	carPosition = getCarPosition();
	check(
		!AT_FLOOR
		|| (carPosition >= -1 && carPosition <= 1)    // floor 1
		|| (carPosition >= FLOOR_2_HIGHT-1 && carPosition <= FLOOR_2_HIGHT+1) // floor 2
		|| (carPosition >= FLOOR_3_HIGHT-1 && carPosition <= FLOOR_3_HIGHT+1),// floor 3
	"env3");


	// fill in your own environment assumption 4
	if (BUTTON1_PRESSED) {
		if (timeSinceButton1Pressed < 0) {
			timeSinceButton1Pressed = 0;
		} else {
			timeSinceButton1Pressed += POLL_TIME;
		}
		check(timeSinceButton1Pressed * portTICK_RATE_MS <= 30000, "env4.1");
		
	} else {
		timeSinceButton1Pressed = -1;
	}
	if (BUTTON2_PRESSED) {
		if (timeSinceButton2Pressed < 0) {
			timeSinceButton2Pressed = 0;
		} else {
			timeSinceButton2Pressed += POLL_TIME;
		}
		check(timeSinceButton2Pressed * portTICK_RATE_MS <= 30000, "env4.2");
		
	} else {
		timeSinceButton2Pressed = -1;
	}
	if (BUTTON3_PRESSED) {
		if (timeSinceButton3Pressed < 0) {
			timeSinceButton3Pressed = 0;
		} else {
			timeSinceButton3Pressed += POLL_TIME;
		}
		check(timeSinceButton3Pressed * portTICK_RATE_MS <= 30000, "env4.2");
		
	} else {
		timeSinceButton3Pressed = -1;
	}
	

    // System requirement 1: if the stop button is pressed, the motor is
	//                       stopped within 1s

	if (STOP_PRESSED) {
	  if (timeSinceStopPressed < 0)
	    timeSinceStopPressed = 0;
      else
	    timeSinceStopPressed += POLL_TIME;

      check(timeSinceStopPressed * portTICK_RATE_MS <= 1000 || MOTOR_STOPPED,
	        "req1");
	} else {
	  timeSinceStopPressed = -1;
	}

    // System requirement 2: the motor signals for upwards and downwards
	//                       movement are not active at the same time

    check(!MOTOR_UPWARD || !MOTOR_DOWNWARD,
          "req2");

	// fill in safety requirement 3
	 check(carPosition >= FLOOR_1_HIGHT && carPosition <= FLOOR_3_HIGHT, "req3");

	
/*	if(!((MOTOR_STOPPED==STOP_PRESSED)||(MOTOR_STOPPED==AT_FLOOR))){
		printf("MOTOR_STOPPED is %d\n", MOTOR_STOPPED);
		printf("STOP_PRESSED is %d\n", STOP_PRESSED);
		printf("AT_FLOOR is %d\n", AT_FLOOR);
	} */
	// fill in safety requirement 4
	if(STOP_PRESSED){
		last_StopPressed = 1;
		check( (MOTOR_STOPPED==STOP_PRESSED)||(MOTOR_STOPPED==AT_FLOOR) , "req4");
	}
	else
	 check( (MOTOR_STOPPED==STOP_PRESSED)||(MOTOR_STOPPED==AT_FLOOR)||last_StopPressed, "req4"); 
	

	// fill in safety requirement 5
	if (MOTOR_STOPPED && AT_FLOOR) {
	  if (timeSinceElevatorAtFloor < 0)
	    timeSinceElevatorAtFloor = 0;
		else
	    timeSinceElevatorAtFloor += POLL_TIME;
	} else {
		check(timeSinceElevatorAtFloor == -1 || timeSinceElevatorAtFloor * portTICK_RATE_MS >= 1000,
	        "req5");
	  timeSinceElevatorAtFloor = -1;
	}

	// fill in safety requirement 6	
	check(abs(lastUpwardSpeed - MOTOR_UPWARD_SPEED) <= 5.0
	&& abs(lastDownwardSpeed - MOTOR_DOWNWARD_SPEED) <= 5.0, "req6");
	lastUpwardSpeed = MOTOR_UPWARD_SPEED;
	lastDownwardSpeed = MOTOR_DOWNWARD_SPEED;
	
	
	vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
	
  }

}

void setupSafety(unsigned portBASE_TYPE uxPriority, Motor * motor) {
  xTaskCreate(safetyTask, "safety", 100, NULL, uxPriority, NULL);
	
}
