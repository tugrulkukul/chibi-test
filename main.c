#include "hal.h"
#include "ch.h"
#include "test.h"

static THD_WORKING_AREA(analogReadWA, 128);

/* ADC Related */

static ADCConfig adccfg = {};

// Create buffer to store ADC results. This is
// one-dimensional interleaved array
#define ADC_BUF_DEPTH 1 // Tek okuma yapacağız her seferinde, continous ve circular buffer tipleri de var
#define ADC_CH_NUM 1    // Tek girişten okuyacağız
static adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM]; // results array

static const ADCConversionGroup adccg = {
   // this 3 fields are common for all MCUs
      // set to TRUE if need circular buffer, set FALSE otherwise
      circular : FALSE,
      // number of channels
      num_channels : ADC_CH_NUM,
      // callback function when conversion ends
      end_cb : NULL,
      //callback function when error appears
      error_cb : NULL,
      //look to datasheet for information about the registers
      // CR1 register content
      cr1 : 0,
      // CR2 register content
      cr2 : ADC_CR2_SWSTART,//?
      // SMRP1 register content
      smpr1 : 0,
      // SMRP2 register content
      smpr2 : 0,
      // SQR1 register content
      sqr1 : ((ADC_CH_NUM - 1) << 20),
      // SQR2 register content
      sqr2 : 0,
      // SQR3 register content. We must select 1 channel
      // For example 2nd channel
      // if we want to select more than 1 channel then simply
      // shift and logic or the values example (ch 15 & ch 10) : (15 | (10 << 5))
      sqr3 : 6
};

/* ADC Related */

/* DAC Related */

/*bu nedir??*/
static const DACConfig dac1cfg1 = {
  init:         1000U,
  datamode:     DAC_DHRM_12BIT_RIGHT
};


static const DACConversionGroup dacgrpcfg1 = {
  num_channels: 1U,
  end_cb:       NULL,
  error_cb:     NULL,
  trigger:      DAC_TRG(0)
};


static const GPTConfig gpt6cfg1 = {
  frequency:    1000000U,
  callback:     NULL,
  cr2:          TIM_CR2_MMS_1,  /* MMS = 010 = TRGO on Update Event.        */
  dier:         0U
};
/* DAC Related */


static void adctodac(void *arg)
{
    uint16_t temp = 0;
    adcStart(&ADCD1, &adccfg);
    dacStart(&DACD1, &dac1cfg1);

    gptStart(&GPTD6, &gpt6cfg1);

    while(!0)
    {
        adcStartConversion(&ADCD1, &adccg, samples_buf, ADC_BUF_DEPTH);
        temp = samples_buf[0];
        //dacPutChannelX(&DACD1, 1U, temp);
        dacStartConversion(&DACD1, &dacgrpcfg1, samples_buf, ADC_BUF_DEPTH);
        gptStartContinuous(&GPTD6, 2U);
        chThdSleepMilliseconds(1);
    }
}

int main(void)
{
	halInit();
	chSysInit();

	thread_t *tp;


	/*analog input*/
	palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG); // this is 10th channel

	/*analog output*/
    palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);

    tp = chThdCreateStatic(analogReadWA, sizeof(analogReadWA), HIGHPRIO, adctodac, NULL);
	chThdWait(tp);

	return 0;
}
