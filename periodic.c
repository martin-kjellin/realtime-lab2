#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

void ecrobot_device_initialize() {
  ecrobot_set_light_sensor_active(NXT_PORT_S1);
}

void ecrobot_device_terminate() {
  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);
}

void user_1ms_isr_type2(void) {}
