#ifndef __DEF_H__
#define __DEF_H__


#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/cpufunc.h>

# define F_CPU 3333333UL
#include <util/delay.h>

#include "i2c.h"



#define LSB	0x01


// светодиод на отладочной плате
//#define LED_ON		PORTB.OUT &= ~PIN5_bm;
//#define LED_OFF		PORTB.OUT |= PIN5_bm;
#define LED_ON		PORTB.OUTCLR = PIN5_bm;
#define LED_OFF		PORTB.OUTSET = PIN5_bm;


// включение питания микросхемы EEPROM
#define PWR_MEM_ON		PORTA.OUT &= ~PIN7_bm;
#define PWR_MEM_OFF		PORTA.OUT |= PIN7_bm;



#define PA6_LOW						!(PORTA.IN & PIN6_bm)
#define PA6_INTERRUPT				PORTA.INTFLAGS & PIN6_bm
#define PA6_CLEAR_INTERRUPT_FLAG	PORTA.INTFLAGS &= PIN6_bm

#define PB2_INTERRUPT				PORTB.INTFLAGS & PIN2_bm
#define PB2_CLEAR_INTERRUPT_FLAG	PORTB.INTFLAGS &= PIN2_bm


// режимы работы T-LOG (TLOG_CFG.mode)
#define MODE_IDLE		0			// после включения питания, до команды START CONTROL
#define MODE_CONTROL	1			// режим контроля температуры
#define MODE_STOPPED	2			// остановлен командой STOP CONTROL или при возникновении исключения



// конфигурация работы T-LOG
typedef struct
{
	uint8_t		mode;
	uint8_t		interval;
	int8_t		top_level;
	int8_t		bottom_level;
} TLOG_CFG;



//=========================   V A R S   =========================//
extern TLOG_CFG	tlog_cfg;

//========================= P R O T O S =========================//
void Init_Memory(void);


#endif	// __DEF_H__