#ifndef C2M_GPIO_H_
#define C2M_GPIO_H_

#include "C2M_options.h"

// learn button
#define SWITCH 13

// pitch inputs
#define CV1_1 19
#define CV2_1 18
#define CV3_1 17
#define CV4_1 16
#define CV5_1 14

// velocity inputs
#define CV1_2 23
#define CV2_2 22
#define CV3_2 21
#define CV4_2 20
#define CV5_2 15

// trigger inputs
#define TR1 10
#define TR2 12
#define TR3 8
#define TR4 6
#define TR5 4

// LED outputs
#define LED1 9
#define LED2 11
#define LED3 7
#define LED4 5
#define LED5 3
#define LEDX 2

#define C2M_GPIO_DEBUG_PIN1 24
#define C2M_GPIO_DEBUG_PIN2 25

#define C2M_GPIO_BUTTON_PINMODE INPUT_PULLUP
#define C2M_GPIO_TRx_PINMODE INPUT_PULLUP
#define C2M_GPIO_LEDx_PINMODE OUTPUT

#endif // C2M_GPIO_H_
