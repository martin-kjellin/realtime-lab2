#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

DeclareTask(MotorcontrolTask);
DeclareTask(EventdispatcherTask);

DeclareEvent(TouchOnEvent);
DeclareEvent(TouchOffEvent);

void ecrobot_device_initialize() {
  nxt_motor_set_speed(NXT_PORT_A, 0, 1);
  nxt_motor_set_speed(NXT_PORT_B, 0, 1);
}

void ecrobot_device_terminate() {
  nxt_motor_set_speed(NXT_PORT_A, 0, 1);
  nxt_motor_set_speed(NXT_PORT_B, 0, 1);
}

void user_1ms_isr_type2(void) {}

TASK(MotorcontrolTask) {
  display_clear(1);
  display_goto_xy(0, 0);
  display_string("Motors off");
  display_update();

  while (TRUE) {
    WaitEvent(TouchOnEvent);
    ClearEvent(TouchOnEvent);
    display_clear(1);
    display_goto_xy(0, 0);
    display_string("Motors on");
    display_update();
    nxt_motor_set_speed(NXT_PORT_A, 50, 1);
    nxt_motor_set_speed(NXT_PORT_B, 50, 1);

    WaitEvent(TouchOffEvent);
    ClearEvent(TouchOffEvent);
    display_clear(1);
    display_goto_xy(0, 0);
    display_string("Motors off");
    display_update();
    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
    nxt_motor_set_speed(NXT_PORT_B, 0, 1);
  }

  TerminateTask();
}

TASK(EventdispatcherTask) {
  int status = 0;

  while (TRUE) {
    if (status == 0 && ecrobot_get_touch_sensor(NXT_PORT_S3) == 1) {
      SetEvent(MotorcontrolTask, TouchOnEvent);
      status = 1;
    }

    if (status == 1 && ecrobot_get_touch_sensor(NXT_PORT_S3) == 0) {
      SetEvent(MotorcontrolTask, TouchOffEvent);
      status = 0;
    }

    systick_wait_ms(10);
  }
}
