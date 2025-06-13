#ifndef __QMI8658C_H_
#define __QMI8658C_H_

#include "inttypes.h"

void qmi8658_init(void);
int8_t qmi8658_is_data_update(void);
void qmi8658_read_sensor_data(float acc[], float gyro[]);


#endif