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

/* Starts and stops the motors according to events generated
   by EventdispatcherTask. */
TASK(MotorcontrolTask) {
  while (TRUE) {
    /* Wait for the touch sensor to be pressed and the light sensor
       to be above the table, and then start the motors. */
    WaitEvent(TouchOnEvent);
    ClearEvent(TouchOnEvent);
    WaitEvent(OnTableEvent);
    ClearEvent(OnTableEvent);
    nxt_motor_set_speed(NXT_PORT_A, 50, 1);
    nxt_motor_set_speed(NXT_PORT_B, 50, 1);

    /* Wait for the touch sensor to be released or the light sensor
       to be outside the table, and then stop the motors. */
    WaitEvent(TouchOffEvent | OffTableEvent);
    ClearEvent(TouchOffEvent);
    ClearEvent(OffTableEvent);
    nxt_motor_set_speed(NXT_PORT_A, 0, 1);
    nxt_motor_set_speed(NXT_PORT_B, 0, 1);
  }

  TerminateTask();
}

/* Generates events for MotorcontrolTask. */
TASK(EventdispatcherTask) {
  static int on_touch = 0;
  static int on_table = 0;

  while (TRUE) {
    /* Display status information. */
    display_clear(1);
    display_goto_xy(0, 0);
    display_string("Touch sensor: ");
    display_int(on_touch, 1);
    display_string("\nLight sensor: ");
    display_int(on_table, 1);
    display_update();

    /* Generate a TouchOnEvent when the touch sensor is pressed. */
    if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 1 && on_touch == 0) {
      SetEvent(MotorcontrolTask, TouchOnEvent);
      on_touch = 1;
    }

    /* Generate a TouchOffEvent when the touch sensor is released. */
    if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 0 && on_touch == 1) {
      SetEvent(MotorcontrolTask, TouchOffEvent);
      on_touch = 0;
      on_table = 0; /* Force an OnTableEvent to be generated later,
                       so that the MotorcontrolTask doesn't hang. */
    }

    /* Generate a OnTableEvent when the light sensor reading goes below
       the threshold. */
    if (ecrobot_get_light_sensor(NXT_PORT_S1) < 700 && on_table == 0) {
      SetEvent(MotorcontrolTask, OnTableEvent);
      on_table = 1;
    }

    /* Generate a OnTableEvent when the light sensor reading goes above
       the threshold. */
    if (ecrobot_get_light_sensor(NXT_PORT_S1) >= 700 && on_table == 1) {
      SetEvent(MotorcontrolTask, OffTableEvent);
      on_table = 0;
      on_touch = 0; /* Force a TouchOnEvent to be generated later,
                       so that the MotorcontrolTask doesn't hang. */
    }

    systick_wait_ms(10);
  }
}
