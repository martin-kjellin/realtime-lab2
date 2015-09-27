#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_DIST 15
#define PRIO_BUTTON 20
#define PRIO_TRUN 18

int colorlimit = 690;
int error = 30;
int speedadjust = 10;

int duration = 100;
int right_speed = 20;
int left_speed = -20;

DeclareCounter(SysTimerCnt);

DeclareTask(MotorcontrolTask);
DeclareTask(ButtonPressTask);
DeclareTask(DisplayTask);
DeclareTask(DistanceTask);
DeclareTask(TurnLeftTask);
DeclareTask(MoveStraightTask);

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
      colorlimit = ecrobot_get_light_sensor(NXT_PORT_S1);
    }
  TerminateTask();
}

TASK (MoveStraightTask){
  int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  if(color <  colorlimit+error && color > colorlimit-error){ //right track
    
    change_driving_command(PRIO_DIST,50, 50,duration);
   
    right_speed = 20;
    left_speed = -20;
  
 }
 TerminateTask();

}

TASK (TurnLeftTask){
  int color = ecrobot_get_light_sensor(NXT_PORT_S1);
  if(color >  colorlimit+error || color < colorlimit-error){
   
    change_driving_command(PRIO_DIST,left_speed, right_speed,100);
    
    if(left_speed > 0){ left_speed += speedadjust; right_speed -= speedadjust;}
    else { left_speed -= speedadjust; right_speed += speedadjust;}
    
    int temp_speed = right_speed;
    right_speed = left_speed;
    left_speed = temp_speed;
    
  }
  //systick_wait_ms(50);
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
  display_string("\nlimit: ");
  display_int(colorlimit, 4);
  display_update();
  ReleaseResource(resource_dc);

  TerminateTask();
}
