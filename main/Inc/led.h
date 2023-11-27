/*
 * led.h
 *
 *  Created on: Nov 24, 2023
 *      Author: Ashwini K
 */

#ifndef LED_H_
#define LED_H_

#define LED_GREEN 8
#define LED_ORANGE 5
#define LED_RED 6
#define LED_BLUE 7

#define DELAY_COUNT_1MS	1250U
#define DELAY_COUNT_1S	(1000U * DELAY_COUNT_1MS)
#define DELAY_COUNT_500MS	(500U * DELAY_COUNT_1MS)
#define DELAY_COUNT_250MS	(250U * DELAY_COUNT_1MS)
#define DELAY_COUNT_125MS	(125U * DELAY_COUNT_1MS)

void led_init_all(void);
void led_on(uint8_t led_on);
void led_off(uint8_t led_on);
void delay(uint32_t count);


#endif /* LED_H_ */
