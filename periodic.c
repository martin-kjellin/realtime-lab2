#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_BUTTON 20

DeclareCounter(SysTimerCnt);
DeclareTask(MotorcontrolTask);
DeclareTask(ButtonPressTask);
DeclareTask(DisplayTask);

struct dc_t {
    U32 duration;
    S32 speed;
    int priority;
} dc = {0, 0, PRIO_IDLE};

void ecrobot_device_initialize() {
  //  ecrobot_set_light_sensor_active(NXT_PORT_S1);
}

void ecrobot_device_terminate() {
  //  ecrobot_set_light_sensor_inactive(NXT_PORT_S1);
}

void user_1ms_isr_type2(void){
  (void) SignalCounter(SysTimerCnt);
}

void change_driving_command(int priority, int speed, int duration) {
	if(priority >= dc.priority)
	{
		dc.speed=speed;
		dc.duration=duration;
		dc.priority=priority;
	}

}

TASK (MotorcontrolTask){
	if(dc.duration > 0) 
	{
		nxt_motor_set_speed(NXT_PORT_A, dc.speed, 1);
    	nxt_motor_set_speed(NXT_PORT_B, dc.speed, 1);
		dc.duration-=50;

		//change_driving_command();
	}
	else
	{
		dc.priority = PRIO_IDLE;
		nxt_motor_set_speed(NXT_PORT_A, 0, 1);
    	nxt_motor_set_speed(NXT_PORT_B, 0, 1);
	}
	TerminateTask();
}

TASK (ButtonPressTask){
	if(ecrobot_get_touch_sensor(NXT_PORT_S3) == 1)
	{
		change_driving_command(PRIO_BUTTON, -50, 1000);
	}
	TerminateTask();
}

TASK (DisplayTask){
	display_clear(1);
    display_goto_xy(0, 0);
    display_string("dc.priority: ");
    display_int(dc.priority, 2);
    display_string("\ndc.speed: ");
    display_int(dc.speed, 4);
    display_string("\ndc.duration: ");
    display_int(dc.duration, 4);
    display_update();
    TerminateTask();
}