/*
 * ATTINY416XNANO.c
 *
 * Created: 20.11.2020 16:02:18
 * Author : Nikolay Semyannov
 */

#include "def.h"


TLOG_CFG	tlog_cfg = {MODE_IDLE, 0, 0, 0};
uint8_t		wakeup_flags = 0x00;



/*********************************************************************************/
ISR(PORTA_PORT_vect)
{
	if(PA6_INTERRUPT)
	{
		PA6_CLEAR_INTERRUPT_FLAG;

		wakeup_flags |= 0x01;	// Energy Harvesting event
	}
}


/*********************************************************************************/
ISR(PORTB_PORT_vect)
{
	if(PB2_INTERRUPT)
	{
		PB2_CLEAR_INTERRUPT_FLAG;

		wakeup_flags |= 0x02;	// RF event
	}
}


/*********************************************************************************/
void PIT_init(void)
{
	uint8_t temp;

	/* Enable oscillator: */
	temp = CLKCTRL.OSC32KCTRLA;
	temp |= CLKCTRL_ENABLE_bm;
	/* Writing to protected register */
	ccp_write_io((void*)&CLKCTRL.OSC32KCTRLA, temp);

	/* Wait for all register to be synchronized */
	while(RTC.STATUS > 0);

	/* Clock Selection: 1KHz from OSCULP32K */
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;

	/* Run in debug: enabled */
	RTC.DBGCTRL = RTC_DBGRUN_bm;

	/* Periodic Interrupt: enabled */
	RTC.PITINTCTRL = RTC_PI_bm;

	RTC.PITCTRLA = RTC_PERIOD_CYC1024_gc | RTC_PITEN_bm;
}



/*********************************************************************************/
#define		HOUR_TRIGGER_	3515
#define		HOUR_TRIGGER__	3600
//#define		HOUR_TRIGGER	3686	// запаздывает на 3 с в час
//#define		HOUR_TRIGGER	3684
//#define		HOUR_TRIGGER	3670
#define		HOUR_TRIGGER	3680
ISR(RTC_PIT_vect)
{
	static uint16_t sec_cnt = 0;


	/* Clear flag by writing '1': */
	RTC.PITINTFLAGS = RTC_PI_bm;

//	LED0_toggle();

	sec_cnt++;


}


/*********************************************************************************/
void SLPCTRL_init(void)
{
	SLPCTRL.CTRLA |= SLPCTRL_SMODE_PDOWN_gc;
	SLPCTRL.CTRLA |= SLPCTRL_SEN_bm;
}


/*********************************************************************************/
void Init_GPIO(void)
{
	// включение питания памяти PA7
	PWR_MEM_OFF
	PORTA.DIR |= PIN7_bm;


	// светодиод на отладочной плате
//	LED_OFF
//	PORTB.DIR |= PIN5_bm;
//	PORTB.DIRSET = PIN5_bm;

	// Init Energy Harvesting Pin Interrupt
	PORTA.DIRCLR = PIN6_bm;
	PORTA.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;

	// Init RF Action Control Pin
	PORTB.DIRCLR = PIN2_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
}


/*********************************************************************************/
/*********************************************************************************/
int main(void)
{
	Init_GPIO();

	I2C_init();

	Init_Memory();

//	PIT_init();
	SLPCTRL_init();

	/* Enable Global Interrupts */
	sei();

	while (1)
	{
		// clear events flags
//		wakeup_flags = 0x00;
		/* Put the CPU in sleep */
		sleep_cpu();

		if(wakeup_flags & 0x01)	// checking for Energy Harvesting event
		{
			_delay_ms(10);

			if(wakeup_flags & 0x02)	// checking for RF event
			{
				wakeup_flags = 0x00;
			}
		}

		/* The PIT interrupt will wake the CPU */
//		if(toggle_flag == true)
		{
//			LED_OFF
//			TIK_L
//			toggle_flag = false;
		}
//		else
		{
//			LED_ON
//			TIK_H
//			toggle_flag = true;
		}

//		LED0_toggle();
//		TIK_L
//		_delay_ms(1);
//		TIK_H
	}

	//// LED init
	//LED_OFF
	//PORTB.DIRSET |= (LSB<<5);
//
	//// Button init
////	PORTB.DIRCLR |= (LSB<<4);
////	PORTB.PIN4CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
//
////	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
////	sei();
//
////	ADC_Init();
//
	//uint8_t test_byte=0;
	//
	//I2C_WriteByte(USER_SPACE_ADDRESS, 0x0100, 0x57);
	//_delay_ms(3);
	//I2C_ReadByte(USER_SPACE_ADDRESS, 0x0100, &test_byte);
//
    //while (1)
    //{
		//if(test_byte==0x57)
			//_delay_ms(500);
		//else
			//_delay_ms(2000);
//
////		temperature_result = MeasureTemperature();
//
		//LED_ON
//
		//if(test_byte==0x57)
			//_delay_ms(500);
		//else
			//_delay_ms(2000);
		//
		//LED_OFF
//
////		I2C_ReadByte(USER_SPACE_ADDRESS, 0x00, &e2prom_data);
//
		////sleep_cnt++;
////
		////if(sleep_cnt>=3)
		////{
			////sleep_mode();
////
			////ATOMIC_BLOCK(ATOMIC_FORCEON)
			////{
				////_delay_ms(1);
			////}
		////}
    //}
}

