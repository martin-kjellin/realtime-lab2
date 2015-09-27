#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_DIST 15
#define PRIO_BUTTON 20
#define PRIO_TRUN 18

int colorlimit = 685; 

DeclareCounter(SysTimerCnt);

DeclareTask(MotorcontrolTask);
DeclareTask(ButtonPressTask);
DeclareTask(DisplayTask);
DeclareTask(DistanceTask);
/*DeclareTask(TurnLeftTask);
DeclareTask(TurnRightTask);
DeclareTask(MoveStraightTask);*/
DeclareTask(FollowTrackTask);

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
      //change_driving_command();
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
      change_driving_command(PRIO_BUTTON, -50, -50, 1000);
    }
  TerminateTask();
}

/*TASK (MoveStraightTask){
  //int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  int duration=100;
  if(ecrobot_get_light_sensor(NXT_PORT_S1) >  colorlimit){ //in the right track
    change_driving_command(PRIO_DIST,20, 20,duration);
  }

 TerminateTask();
}*/

/*TASK (TurnLeftTask){
  //int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  int duration=100;

  if(ecrobot_get_light_sensor(NXT_PORT_S1) <  colorlimit){ // in the wrong track change the direction   
    change_driving_command(PRIO_DIST, 0, 15,duration);
  }
 TerminateTask();   
}*/

/*TASK (TurnRightTask){
  //int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  int duration=100;

  if(ecrobot_get_light_sensor(NXT_PORT_S1) <  colorlimit){ // didn't find the right track and turn right
    change_driving_command(PRIO_DIST, 15, 0,duration);
  }

 TerminateTask();
}*/

TASK (FollowTrackTask) {
  int right_speed = 0;
  int left_speed = 30;
  int duration = 1000;
  while (ecrobot_get_light_sensor(NXT_PORT_S1) < colorlimit) {
    change_driving_command(PRIO_DIST, left_speed, right_speed, duration);
    duration +=1000;
    int temp_speed = right_speed*1.02;
    right_speed = left_speed*1.02;
    left_speed = temp_speed;
    //systick_wait_ms(30); 
    nxt_motor_set_speed(NXT_PORT_A, dc.speedright, 1);
    nxt_motor_set_speed(NXT_PORT_B, dc.speedleft, 1);
  }

  change_driving_command(PRIO_DIST, 30, 30, 1000);

  TerminateTask();
}

/*TASK (DistanceTask){
  int dis = ecrobot_get_sonar_sensor(NXT_PORT_S2);
  if(dis >= 0)
    {
      dis-=4;
      int speed = (dis-20)*3;
      if(speed > 50) speed = 50;
      else if(speed < 10 && speed > 0) speed=10;
      else if(speed > -10 && speed < 0) speed-=10;
      else if(speed < -50 ) speed = -50;
      
      change_driving_command(PRIO_DIST, speed, speed, 100);
    }
  TerminateTask();
  }*/

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
  display_update();
  ReleaseResource(resource_dc);

  TerminateTask();
}
