/*
 * lmt84lp.h
 *
 *  Created on: 24 Jan 2025
 *      Author: e2203130
 */

#ifndef SENSORS_LMT84LP_H_
#define SENSORS_LMT84LP_H_

void LMT84LP_init();
void LMT84LP_read(char *buffer, int buffer_size);

#endif /* SENSORS_LMT84LP_H_ */
