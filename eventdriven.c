#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

DeclareTask(MotorcontrolTask);
DeclareTask(EventdispatcherTask);

DeclareEvent(TouchOnEvent);
DeclareEvent(TouchOffEvent);

DeclareEvent(OnTableEvent);
DeclareEvent(OffTableEvent);

void ecrobot_device_initialize() {
  ecrobot_set_light_sensor_active(NXT_PORT_S1);

  nxt_motor_set_speed(NXT_PORT_A, 0, 1);
  nxt_motor_set_speed(NXT_PORT_B, 0, 1);
}

void ecrobot_device_terminate() {
  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);

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
    WaitEvent(OnTableEvent);
    ClearEvent(OnTableEvent);
    display_clear(1);
    display_goto_xy(0, 0);
    display_string("Motors on");
    display_update();
    nxt_motor_set_speed(NXT_PORT_A, 50, 1);
    nxt_motor_set_speed(NXT_PORT_B, 50, 1);

    WaitEvent(TouchOffEvent | OffTableEvent);
    ClearEvent(TouchOffEvent);
    ClearEvent(OffTableEvent);
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
  int on_touch = 0;
  int on_table = 0;

  while (TRUE) {
    display_clear(1);
    display_goto_xy(0, 0);
    display_string("on_touch: ");
    display_int(on_touch, 1);
    display_string("\non_table: ");
    display_int(on_table, 1);
    display_update();

    if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 1 && ecrobot_get_light_sensor(NXT_PORT_S1) < 700) {
      SetEvent(MotorcontrolTask, TouchOnEvent);
      SetEvent(MotorcontrolTask, OnTableEvent);
      on_touch = 1;
      on_table = 1;
    }

    if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 0 &&  ecrobot_get_light_sensor(NXT_PORT_S1) < 700 ) {
      SetEvent(MotorcontrolTask, TouchOffEvent);
      SetEvent(MotorcontrolTask, OnTableEvent);
      on_touch = 0;
      on_table = 1;
    }

    if ( ecrobot_get_touch_sensor(NXT_PORT_S3) == 1 &&ecrobot_get_light_sensor(NXT_PORT_S1) >= 700) {
      SetEvent(MotorcontrolTask, TouchOnEvent);
      SetEvent(MotorcontrolTask, OffTableEvent);
      on_touch = 1;
      on_table = 0;
    }

    if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 0 && ecrobot_get_light_sensor(NXT_PORT_S1) >= 700) {
      SetEvent(MotorcontrolTask, TouchOffEvent);
      SetEvent(MotorcontrolTask, OffTableEvent);
      on_touch = 0;
      on_table = 0;
    }

    systick_wait_ms(10);
  }
}
