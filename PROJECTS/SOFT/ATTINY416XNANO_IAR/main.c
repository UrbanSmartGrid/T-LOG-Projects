#include "def.h"




TLOG_CFG	tlog_cfg = {MODE_IDLE, 0, 0, 0};
//!!!DEBUG
//TLOG_CFG	tlog_cfg = {MODE_CONTROL, 2, 3, 4};

/**********************************************************************************/
// задержка в мс для OSC20M = 20 MHz  & Prescale = 6
/*inline*/ void delay_ms(unsigned int delay)
{
	while(delay)
	{
		__delay_cycles(3378);
		delay--;
	}
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

//	// Init Energy Harvesting Pin Interrupt
//	PORTA.DIRCLR = PIN6_bm;
//	PORTA.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc;
//
//	// Init RF Action Control Pin
//	PORTB.DIRCLR = PIN2_bm;
//	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
}


/*********************************************************************************/
void PIT_init(void)
{
//	uint8_t temp;

	/* Enable oscillator: */
//	temp = CLKCTRL.OSC32KCTRLA;
//	temp |= CLKCTRL_ENABLE_bm;
//	/* Writing to protected register */
//	CCP = 0xD8;
//	CLKCTRL.OSC32KCTRLA = temp;
//
//	/* Wait for all register to be synchronized */
//	while(RTC.STATUS > 0);

	/* Clock Selection: 1KHz from OSCULP32K */
	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;

	/* Run in debug: enabled */
	RTC.DBGCTRL = RTC_DBGRUN_bm;

	/* Periodic Interrupt: enabled */
	RTC.PITINTCTRL = RTC_PI_bm;

	RTC.PITCTRLA = RTC_PERIOD_CYC1024_gc | RTC_PITEN_bm;
}


/*********************************************************************************/
#define	COMPARE_CFG(cfg1, cfg2)		((cfg1.mode == cfg2.mode)&&\
										(cfg1.interval == cfg2.interval)&&\
										(cfg1.top_level == cfg2.top_level)&&\
										(cfg1.bottom_level == cfg2.bottom_level))
#define	TEN_SECONDS			10
#define	THIRTY_SECONDS		30

#pragma vector = RTC_PIT_vect
__interrupt void PIT_interrupt(void)
{
//	static uint8_t flag = 0;	// only for debug
	static uint8_t	_10sec_interval = 0;				// счётчик для отсчёта 10-секундных интервалов для проверки конфигурации
	static uint8_t	_30sec_interval = 0;				// счётчик для отсчёта 30-секундных интервалов 
	static uint8_t	temperature_control_interval = 0;
	TLOG_CFG memory_tlog_cfg;
	
	
	/* Clear flag by writing '1': */
	RTC.PITINTFLAGS = RTC_PI_bm;
	
	_10sec_interval++;
	
	if(_10sec_interval == TEN_SECONDS)
	{	
		_10sec_interval = 0;
		
		PWR_MEM_ON
			
		I2C_ReadBytes(USER_SPACE_ADDRESS, CFG_ADDRESS, (uint8_t*)(&memory_tlog_cfg), 4);
		
		PWR_MEM_OFF
			
//		printf("TIK 10 sec\n");			
		
		if(!COMPARE_CFG(memory_tlog_cfg, tlog_cfg))	// конфигурация изменена
		{
			tlog_cfg.mode			= memory_tlog_cfg.mode;
			tlog_cfg.interval		= memory_tlog_cfg.interval;
			tlog_cfg.top_level		= memory_tlog_cfg.top_level;
			tlog_cfg.bottom_level	= memory_tlog_cfg.bottom_level;
			
			if(tlog_cfg.mode == MODE_BLOCKED)
			{
				// останавливем микроконтроллер в режиме POWER DOWN
				/* Periodic Interrupt: disabled */
				RTC.PITINTCTRL &= ~RTC_PI_bm;
			}

			
			_30sec_interval = 0;
			temperature_control_interval = 0;
			
//			printf("CFG was changed\n");
						
		}
	}
	
	
	if((tlog_cfg.mode == MODE_IDLE) || (tlog_cfg.mode == MODE_STOPPED) || (tlog_cfg.mode == MODE_STOPPED)) // do nothing
	{
//		if(flag==0)
//		{
//			flag = 1;
//			PWR_MEM_ON
//		}
//		else
//		{
//			flag = 0;
//			PWR_MEM_OFF
//		}
	}
	else	// tlog_cfg.mode == MODE_CONTROL
	{
		_30sec_interval++;
	
		if(_30sec_interval == THIRTY_SECONDS)
		{
			_30sec_interval = 0;
			
			temperature_control_interval++;
			
			if(temperature_control_interval == tlog_cfg.interval)
			{
				temperature_control_interval = 0;
				
				// проводим измерения
			}
		}
	}
}


//********************************************************************************//
void main(void)
{
	Init_GPIO();
	
	I2C_init();
	
	Init_Memory();
	
	SLPCTRL_init();
	
	PIT_init();

	/* Enable Global Interrupts */
	__enable_interrupt();
	
	
	while(1)
	{	
		__sleep();
	}
}
