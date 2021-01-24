#ifndef __I2C_H__
#define __I2C_H__

#include <avr/io.h>

#define		PIN_I2C_SCL		PIN0_bm		// PB0
#define		PIN_I2C_SDA		PIN1_bm		// PB1


#define I2C_READ			1
#define I2C_WRITE			0
#define I2C_WAIT_TIMEOUT	0xFF

#define I2C_STATE_IS_HIGH(x)	((TWI0.MSTATUS & (x)) == (x))
#define I2C_STATE_IS_LOW(x)		((TWI0.MSTATUS & (x)) != (x))
#define I2C_BUS_STATE			(TWI0.MSTATUS & TWI_BUSSTATE_gm)

#define I2C_BUS_NOT_IDLE		I2C_STATE_IS_LOW(TWI_BUSSTATE_IDLE_gc)
#define I2C_BUS_IDLE			I2C_STATE_IS_HIGH(TWI_BUSSTATE_IDLE_gc)

#define I2C_BUS_NOT_BUSY		I2C_STATE_IS_LOW(TWI_BUSSTATE_BUSY_gc)
#define I2C_BUS_BUSY			I2C_STATE_IS_HIGH(TWI_BUSSTATE_BUSY_gc)

#define I2C_BUS_NOT_OWNER		I2C_STATE_IS_LOW(TWI_BUSSTATE_OWNER_gc)
#define I2C_BUS_OWNER			I2C_STATE_IS_HIGH(TWI_BUSSTATE_OWNER_gc)

#define I2C_NOT_CLOCKHOLD		I2C_STATE_IS_LOW(TWI_CLKHOLD_bm)
#define I2C_CLOCKHOLD			I2C_STATE_IS_HIGH(TWI_CLKHOLD_bm)

#define I2C_NOT_BUSERR			I2C_STATE_IS_LOW(TWI_BUSERR_bm)
#define I2C_BUSERR				I2C_STATE_IS_HIGH(TWI_BUSERR_bm)

#define I2C_NOT_ARBLOST			I2C_STATE_IS_LOW(TWI_ARBLOST_bm)
#define I2C_ARBLOST				I2C_STATE_IS_HIGH(TWI_ARBLOST_bm)


typedef enum {	RETURN_OK=0,
	RETURN_FAILED=1,
	RETURN_NO_SLAVE=2,
	RETURN_BUS_ERROR=3,
	RETURN_BUS_ARBLOST=4,
RETURN_BUS_BUSY=5} RETURN_RESULT;


#define		USER_SPACE_ADDRESS		0b01010011		// E2 = 0
#define		SYSTEM_SPACE_ADDRESS	0b01010111		// E2 = 1


#define		CFG_ADDRESS					0
#define		SYSTEM_SSS_REG				0
#define		SYSTEM_CFG_REG				2320
#define		SYSTEM_CTRL_REG				2336
#define		SYSTEM_UID_START_ADDRESS	2324
#define		SYSTEM_I2C_LOCK				2048

#define		UID_LENGTH	8	// длина UID == 8 байтов, младший по адресу SYSTEM_UID_START_ADDRESS


void I2C_init(void);
RETURN_RESULT I2C_ReadByte(uint8_t chip_address, uint16_t reg_address, uint8_t *data);
//RETURN_RESULT I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size);
RETURN_RESULT I2C_WriteByte(uint8_t chip_address, uint16_t reg_address, uint8_t data);
//RETURN_RESULT I2C_WriteBytes(uint8_t chip_address, uint8_t reg_address, uint8_t data[], uint8_t size);
void I2C_WriteBytes(uint8_t chip_address, uint8_t reg_address, uint8_t data[], uint8_t size);
void I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size);


#endif	// __I2C_H__