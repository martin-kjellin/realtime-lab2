#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_DIST 15
#define PRIO_BUTTON 20
#define PRIO_TRUN 18

int colormax = 690;
int colormin = 0;
int offset=0;

int speedmax = 50;
int speedmin = 10;

int duration = 100;
int right_speed = 20;
int left_speed = -20;
int counter = 0;

DeclareCounter(SysTimerCnt);

DeclareTask(MotorcontrolTask);
DeclareTask(ButtonPressTask);
DeclareTask(DisplayTask);
DeclareTask(DistanceTask);
DeclareTask(MoveStraightTask);
DeclareTask(DistanceTask);
DeclareResource(resource_dc);

struct dc_t {
  U32 duration;
  S32 speedleft;
  S32 speedright;
  int priority;
} dc = {0, 0, 0, PRIO_IDLE};

void ecrobot_device_initialize() {
  ecrobot_set_light_sensor_active(NXT_PORT_S1);
  ecrobot_init_sonar_sensor(NXT_PORT_S2);
}

void ecrobot_device_terminate() {
  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);
  ecrobot_term_sonar_sensor(NXT_PORT_S2);
}


void user_1ms_isr_type2(void){
  (void) SignalCounter(SysTimerCnt);
}

void change_driving_command(int priority, int speedleft, int speedright, int duration) {
	
  GetResource(resource_dc);
  if(priority >= dc.priority)
    {
      dc.speedleft=speedleft;
      dc.speedright=speedright;
      dc.duration=duration;
      dc.priority=priority;
    }
  ReleaseResource(resource_dc);
}

TASK (MotorcontrolTask){

  GetResource(resource_dc);
  if(dc.duration > 0) 
    {
      nxt_motor_set_speed(NXT_PORT_A, dc.speedright, 1);
      nxt_motor_set_speed(NXT_PORT_B, dc.speedleft, 1);
      dc.duration-=50;
     
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

TASK (ButtonPressTask){
  if(ecrobot_get_touch_sensor(NXT_PORT_S3) == 1)
    {
      if(counter == 0){
	colormax = ecrobot_get_light_sensor(NXT_PORT_S1);
	counter=1;
      }

      else {
	colormin = ecrobot_get_light_sensor(NXT_PORT_S1);
	counter=0;
	offset = (colormax+colormin)/2;
      }
     
    }

  TerminateTask();
}

TASK (MoveStraightTask){
  
  int kp=10;
  int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  right_speed = kp*(color-offset)+30;
  left_speed = -right_speed;
  if(right_speed > speedmax) right_speed = speedmax;
  if(right_speed < speedmin) right_speed = speedmin;
  if(left_speed > speedmax) left_speed = speedmax;
  if(left_speed < speedmin) left_speed = speedmin;

  change_driving_command(PRIO_DIST, left_speed, right_speed, duration);
  TerminateTask();

}

TASK (DistanceTask){
  int dis = ecrobot_get_sonar_sensor(NXT_PORT_S2);
  if(dis >= 0)
    {
      dis-=4;
      if(dis <=40){
	change_driving_command(PRIO_DIST, 0, 0, 100);
      }

    }
  TerminateTask();
  }

TASK (DisplayTask){

  GetResource(resource_dc);
  display_clear(1);
  display_goto_xy(0, 0);
  display_string("color: ");
  display_int(ecrobot_get_light_sensor(NXT_PORT_S1), 4);
  display_string("\ndc.priority: ");
  display_int(dc.priority, 2);
  display_string("\nlspeed: ");
  display_int(dc.speedleft, 4);
  display_string("\nrspeed: ");
  display_int(dc.speedright, 4);
  display_string("\ndc.dura: ");
  display_int(dc.duration, 4);
  display_string("\ndis: ");
  display_int(ecrobot_get_sonar_sensor(NXT_PORT_S2)-4, 3);
  display_string("\ncmax: ");
  display_int(colormax, 4);
  display_string("\ncmin: ");
  display_int(colormin, 4);
  display_update();
  ReleaseResource(resource_dc);

  TerminateTask();
}
