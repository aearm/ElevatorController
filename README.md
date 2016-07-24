# ElevatorController

an elevator, moving people up and down between three floors. A picture of the situation is shown on the right.
In the elevator car, there are four buttons: buttons 1, 2, 3 to go to the respective floor, and a stop button to stop the elevator. On each floor, there is a button to call the elevator. Furthermore, the elevator has two sensors, the first indicating if the elevator is on a floor or in between two floors, the second one indicating if the elevator's door is closed or not. There is also a linear position sensor for measuring the position of the elevator in the elevator shaft.

The elevator is moved up and down by a motor that is on the roof of the building, at a maximum speed of 50cm/s. The motor is controlled by two signals, one for moving the elevator up and one for moving it down; both signals are driven using pulse-width modulation (PWM) to enable smooth acceleration and deceleration of the elevator car. The distance between two floors is 400cm.

The goal of this project is to implement and test the software controlling the elevator.
