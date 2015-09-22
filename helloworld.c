#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

DeclareTask(HelloworldTask);

void ecrobot_device_initialize() {
  ecrobot_set_light_sensor_active(NXT_PORT_S1);
}

void ecrobot_device_terminate() {
  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);
}

void user_1ms_isr_type2(void) {}

TASK(HelloworldTask) {
  display_string("Hello World!");
  display_update();
  systick_wait_ms(3000);

  int reading = 0;

  while (TRUE) {
    reading = ecrobot_get_light_sensor(NXT_PORT_S1);
    display_clear(1);
    display_goto_xy(0, 0);
    display_int(reading, 4);
    display_update();
    systick_wait_ms(100);
  }

  TerminateTask();
}
