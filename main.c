#include "hal.h"
#include "ch.h"
#include "test.h"

static PWMConfig pwmcfg = {
  2000000, /* 200Khz PWM clock frequency*/
  4, /* PWM period of 1024 ticks ~ 0.005 second */
  NULL, /* No callback */
  /* Only channel 1 enabled */
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}
  }
};


static THD_WORKING_AREA(waThread1, 128);
static THD_WORKING_AREA(waThread2, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetPad(GPIOD, GPIOD_LED3);       /* Orange.  */
    chThdSleepMilliseconds(500);
    palClearPad(GPIOD, GPIOD_LED3);     /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}

static THD_FUNCTION(Thread2, arg) {
    (void)arg;
    //chRegSetThreadName("pwm-test");
    pwmcnt_t duty = 2, period = 40;
    pwmEnableChannel(&PWMD1, 0, (pwmcnt_t)duty);
    while (true) {
        //pwmEnableChannel(&PWMD1, 0, (pwmcnt_t)duty);
        pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH (&PWMD1, 5000)); /* 50% */
        pwmChangePeriod(&PWMD1, (pwmcnt_t)period);
        period += 1;
        chThdSleepMilliseconds(200);
    }

}

int main(void)
{
	halInit();
	chSysInit();


    pwmStart(&PWMD1, &pwmcfg);
    palSetPadMode(GPIOE, 9, PAL_MODE_ALTERNATE(1));


    /* serial port */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);


	while(true){
        chThdSleepMilliseconds(500);
	}

	return 0;
}
