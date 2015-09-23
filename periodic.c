#include <stdlib.h>
#include "kernel.h"
#include "kernel_id.h"
#include "ecrobot_interface.h"

#define PRIO_IDLE 10
#define PRIO_BUTTON 20

DeclareCounter(SysTimerCnt);

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

}
