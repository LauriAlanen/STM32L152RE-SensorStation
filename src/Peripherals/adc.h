/*
 * adc.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef PERIPHERALS_ADC_H_
#define PERIPHERALS_ADC_H_

#define ADC_MAX 4096
#define ADC_STEP_SIZE_U 0.000805664f // 3.3V / 4096

#define CHANNEL_MASK 0x1F

void ADC_init();

#endif /* PERIPHERALS_ADC_H_ */
