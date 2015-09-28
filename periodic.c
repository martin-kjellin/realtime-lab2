#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_DIST 15
#define PRIO_BUTTON 20

DeclareCounter(SysTimerCnt);

DeclareTask(MotorcontrolTask);
DeclareTask(ButtonpressTask);
DeclareTask(DisplayTask);
DeclareTask(DistanceTask);

DeclareResource(resource_dc);

/* Structure storing driving commands from the sensing tasks. */
struct dc_t {
  U32 duration;
  S32 speed;
  int priority;
} dc = {0, 0, PRIO_IDLE};

void ecrobot_device_initialize() {
  ecrobot_init_sonar_sensor(NXT_PORT_S2);
}

void ecrobot_device_terminate() {
  ecrobot_term_sonar_sensor(NXT_PORT_S2);
}

void user_1ms_isr_type2(void){
  (void) SignalCounter(SysTimerCnt);
}

/* Stores a new driving command, if it has a higher priority than
   the current one. */
void change_driving_command(int priority, int speed, int duration) {
  GetResource(resource_dc);

  if (priority >= dc.priority)
    {
      dc.speed = speed;
      dc.duration = duration;
      dc.priority = priority;
    }

  ReleaseResource(resource_dc);
}

/* Sets the speed of the motors according to the stored
   driving command, if the time of the command has not
   elapsed. Otherwise, sets the priority of the driving
   command struct to the lowest possible, in order to
   prepare it for receiving a new command. */
TASK(MotorcontrolTask) {
  GetResource(resource_dc);

  if (dc.duration > 0) 
    {
      nxt_motor_set_speed(NXT_PORT_A, dc.speed, 1);
      nxt_motor_set_speed(NXT_PORT_B, dc.speed, 1);
      dc.duration -= 50;
    }
  else
    {
      nxt_motor_set_speed(NXT_PORT_A, 0, 1);
      nxt_motor_set_speed(NXT_PORT_B, 0, 1);
      dc.priority = PRIO_IDLE;
    }

  ReleaseResource(resource_dc);

  TerminateTask();
}

/* Stores a new driving command making the car go
   backwards, if the touch sensor is pressed. */
TASK(ButtonpressTask) {
  if (ecrobot_get_touch_sensor(NXT_PORT_S3) == 1)
    {
      change_driving_command(PRIO_BUTTON, -50, 1000);
    }
  TerminateTask();
}

/* Displays information about the current driving commands
   and the current distance. */
TASK(DisplayTask) {
  GetResource(resource_dc);

  display_clear(1);
  display_goto_xy(0, 0);

  display_string("dc.priority: ");
  display_int(dc.priority, 2);

  display_string("\ndc.speed: ");
  display_int(dc.speed, 4);

  display_string("\ndc.dura: ");
  display_int(dc.duration, 4);

  /* Displays the current distance reading, compensating for
     inaccuracies. */
  display_string("\ndistance: ");
  display_int(ecrobot_get_sonar_sensor(NXT_PORT_S2) - 4, 3);

  display_update();

  ReleaseResource(resource_dc);

  TerminateTask();
}

/* Keeps a constant distance of 20 cm to another object. */
TASK(DistanceTask) {
  int dis = ecrobot_get_sonar_sensor(NXT_PORT_S2);

  if (dis >= 0)
    {
      /* Compensate for inaccuracies in measurements. */
      dis -= 4;

      /* Set speed and direction according to the distance
         to the object. */
      int speed = (dis - 20) * 3;

      /* Keep the speed within certain limits. */
      if (speed > 50) speed = 50;
      else if (speed < 10 && speed > 0) speed= 10;
      else if (speed > -10 && speed < 0) speed= -10;
      else if (speed < -50 ) speed = -50;
     
      change_driving_command(PRIO_DIST, speed, 100);
    }

  TerminateTask();
}
