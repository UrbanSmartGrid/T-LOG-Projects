#include "def.h"


volatile int8_t temperature_result;


/*********************************************************************************/
void ADC_Init(void)
{
	// Configure the internal voltage reference to 1.1V
	VREF.CTRLA = VREF_ADC0REFSEL_1V1_gc;
	ADC0.CTRLC =	ADC_PRESC_DIV32_gc			/* CLK_PER divided by 32 */
	| ADC_REFSEL_INTREF_gc		/* Internal reference */
	| (0 << ADC_SAMPCAP_bp);	/* Sample Capacitance Selection: disabled */

	ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc;		/* Temp sensor */

	ADC0.CTRLA = (1 << ADC_ENABLE_bp)			/* ADC Enable: enabled */
	| 0 << ADC_FREERUN_bp			/* ADC Freerun mode: disabled */
	| ADC_RESSEL_10BIT_gc			/* 10-bit mode */
	| 0 << ADC_RUNSTBY_bp;			/* Run standby mode: disabled */
}


/*********************************************************************************/
int16_t	SampleKelvinTemperature(void)
{
	ADC0_COMMAND = ADC_STCONV_bm;
	while(ADC0_COMMAND == ADC_STCONV_bm);

	int8_t		sigrow_offset = SIGROW.TEMPSENSE1;	// Read signed value from signature row
	uint8_t		sigrow_gain = SIGROW.TEMPSENSE0;	// Read unsigned value from signature row
	uint16_t	adc_reading = ADC0.RES;				// ADC conversion result with 1.1 V internal reference

	uint32_t	temperature_in_K = adc_reading - sigrow_offset;
	temperature_in_K *= sigrow_gain;				// Result might overflow 16 bit variable (10bit+8bit)
	temperature_in_K += 0x80;						// Add 1/2 to get correct rounding on division below
	temperature_in_K >>= 8;							// Divide result to get Kelvin


	return temperature_in_K;
}


/*********************************************************************************/
#define TEMPERATURE_SAMPLE_NUMBER	8
int8_t MeasureTemperature(void)
{
	int32_t	temperature = 0;


	for(uint8_t sample_idx=0; sample_idx<TEMPERATURE_SAMPLE_NUMBER; sample_idx++)
		temperature += SampleKelvinTemperature();

	temperature >>= 3;

	temperature -= 273;


	return temperature;	// in Celsius
}