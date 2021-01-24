#include "def.h"


/*********************************************************************************/
void I2C_init(void)
{
	PORTB.DIRSET = PIN_I2C_SCL;
	PORTB.DIRCLR = PIN_I2C_SDA;

	// Use max SDA setup and hold times to be safe
	TWI0.CTRLA = TWI_SDAHOLD_500NS_gc | TWI_SDASETUP_8CYC_gc;

	// set MBAUD register for 100kHz (96,1 kHz)
	TWI0.MBAUD = 9;

	TWI0.MCTRLA = 1 << TWI_ENABLE_bp		/* Enable TWI Master: enabled */
	| 0 << TWI_QCEN_bp						/* Quick Command Enable: disabled */
	| 0 << TWI_RIEN_bp						/* Read Interrupt Enable: disabled */
	| 1 << TWI_SMEN_bp						/* Smart Mode Enable: enabled */
	| TWI_TIMEOUT_DISABLED_gc				/* Bus Timeout Disabled */
	| 0 << TWI_WIEN_bp;						/* Write Interrupt Enable: disabled */

	// Force TWI state machine into IDLE state
	TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc ;
	TWI0.MSTATUS = (TWI_RIF_bm | TWI_WIF_bm) ;

	/* Purge MADDR and MDATA */
	TWI0.MCTRLB = TWI_FLUSH_bm ;
}


/*********************************************************************************/
RETURN_RESULT I2C_RawStart(uint8_t deviceAddr, int Direction)
{
	volatile uint8_t timeout;


	deviceAddr = (deviceAddr << 1) | Direction; // TRUE - read, FALSE - write

	if (I2C_BUS_NOT_BUSY)
	{
		for (volatile uint16_t arbLoop = 0x04; arbLoop > 0; arbLoop--)
		{
			TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);
			TWI0.MADDR = deviceAddr;

			if (Direction)
			{
				// addressRead
				for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_RIF_bm) && (timeout > 0); timeout--)
				{
				}
			}
			else
			{
				// addressWrite
				for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_WIF_bm) && (timeout > 0); timeout--)
				{
				}
			}

			if (I2C_ARBLOST)
			{
				// Arbitration lost - keep retrying.
				continue;
			}

			if (I2C_BUSERR)
			{
				// Bus error - abort.
				return(RETURN_FAILED);		// RETURN_BUS_ERROR);
			}

			if (I2C_STATE_IS_LOW(TWI_RXACK_bm))
			{
				// Slave responding.
				return(RETURN_OK);		// RETURN_OK);
			}
			else
			{
				// Slave not responding - abort.
				return(RETURN_FAILED);		// RETURN_NO_SLAVE);
			}
		}
	}
	else
	{
		return(RETURN_FAILED);		// RETURN_BUS_BUSY);
	}

	return(RETURN_FAILED);		// RETURN_BUS_ARBLOST);
}


/*********************************************************************************/
RETURN_RESULT I2C_RawRead(uint8_t ACK, uint8_t *data)							// ACK=1 send ACK ; ACK=0 send NACK
{
	volatile uint8_t timeout;


	if (I2C_BUS_OWNER)
	{
		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--)
		{
			// Wait until we have a clock hold.
		}

		if (timeout == 0)
		{
			// Even though we own the bus, we aren't holding the clock.
			return(RETURN_FAILED);
		}

		TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);

		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_RIF_bm) && (timeout > 0); timeout--)
		{
			// Wait until RIF set
		}

		if (timeout == 0)
		{
			// Timeout on waiting for RIF signal.
			return(RETURN_FAILED);
		}

		*data = TWI0.MDATA;
		if	(ACK == 1)	{TWI0.MCTRLB &= ~(1 << TWI_ACKACT_bp);}		// si ACK=1 mise à 0 ACKACT => action send ack
		else			{TWI0.MCTRLB |= (1 << TWI_ACKACT_bp);	}	// sinon (ACK=0) => mise à 1 ACKACT => nack préparé pour actionstop

		//		if (I2C_ARBLOST) {
		//			// Arbitration lost - keep retrying.
		//			continue;
		//		}

		if (I2C_BUSERR)
		{
			// Bus error - abort.
			return(RETURN_FAILED);		// RETURN_BUS_ERROR);
		}

		if (I2C_STATE_IS_LOW(TWI_RXACK_bm))
		{
			// Slave responding.
			return(RETURN_OK);		// RETURN_OK);
		}
		else
		{
			// Slave not responding - abort.
			return(RETURN_FAILED);		// RETURN_NO_SLAVE);
		}
	}

	return(RETURN_FAILED);
}


/*********************************************************************************/
RETURN_RESULT I2C_RawWrite(uint8_t write_data)
{
	volatile uint8_t timeout;

	if (I2C_BUS_OWNER)
	{
		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--)
		{
			// Wait until we have a clock hold.
		}

		if (timeout == 0)
		{
			// Even though we own the bus, we aren't holding the clock.
			return(RETURN_FAILED);
		}

		TWI0.MDATA = write_data;
		// while (!((TWI0.MSTATUS & TWI_WIF_bm) | (TWI0.MSTATUS & TWI_RXACK_bm))) ;		//Wait until WIF set and RXACK cleared

		for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_WIF_bm) && (timeout > 0); timeout--)
		{
		}

		//		if (I2C_ARBLOST) {
		//			// Arbitration lost - keep retrying.
		//			continue;
		//		}

		if (I2C_BUSERR)
		{
			// Bus error - abort.
			return(RETURN_FAILED);		// RETURN_BUS_ERROR);
		}

		if (I2C_STATE_IS_LOW(TWI_RXACK_bm))
		{
			// Slave responding.
			return(RETURN_OK);		// RETURN_OK);
		}
		else
		{
			// Slave not responding - abort.
			return(RETURN_FAILED);		// RETURN_NO_SLAVE);
		}
	}

	return(RETURN_FAILED);
}


/*********************************************************************************/
RETURN_RESULT I2C_RawStop(void)
{
	volatile uint8_t timeout;

	for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--)
	{
		// Wait until we have a clock hold.
	}

	if (timeout == 0)
	{
		// Even though we own the bus, we aren't holding the clock.
		return(RETURN_FAILED);
	}

	//TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
	TWI0.MCTRLB = TWI_MCMD_STOP_gc;

	for (timeout = I2C_WAIT_TIMEOUT; I2C_STATE_IS_LOW(TWI_CLKHOLD_bm) && (timeout > 0); timeout--)
	{
		// Wait until we have a clock hold.
	}

	if (I2C_BUSERR)
	{
		// A bit of a fudge - I'm getting bus errors for some reason.
		// Doesn't seem to affect anything though.
		TWI0.MSTATUS = TWI_BUSERR_bm;
	}

	return(RETURN_OK);
}


/*********************************************************************************/
void I2C_RawForceStop(void)
{
	I2C_RawStop();
	TWI0.MSTATUS |= TWI_BUSSTATE_IDLE_gc;	// Force TWI state machine into IDLE state
	TWI0.MCTRLB |= TWI_FLUSH_bm;			// Purge MADDR and MDATA
}


/*********************************************************************************/
RETURN_RESULT I2C_ReadByte(uint8_t chip_address, uint16_t reg_address, uint8_t *data)
{
	if(I2C_BUS_NOT_BUSY)
	{
		if(I2C_RawStart(chip_address, I2C_WRITE))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawWrite(reg_address>>8))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawWrite(reg_address))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawStart(chip_address, I2C_READ))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawRead(0, data))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		I2C_RawStop();

		return(RETURN_OK);
	}
	else
		return(RETURN_BUS_BUSY);
}


///*********************************************************************************/
//RETURN_RESULT I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size)
//{
	//if (I2C_BUS_NOT_BUSY)
	//{
		//if (I2C_RawStart(address, I2C_WRITE))
		//{
			//I2C_RawStop();
			//return(RETURN_FAILED);
		//}
//
		//if (I2C_RawWrite(reg))
		//{
			//I2C_RawStop();
			//return(RETURN_FAILED);
		//}
//
		//if (I2C_RawStart(address, I2C_READ))
		//{
			//I2C_RawStop();
			//return(RETURN_FAILED);
		//}
//
		//for (uint8_t index = 0x00; index < size; index++)
		//{
			//// For the last packet received we want to send a NACK to stop it from sending any more.
			//// (index + 1 == size) ? 1 : 0
			//if (I2C_RawRead((index + 1 == size) ? 0 : 1, data++))
			//{
				//I2C_RawStop();
				//return(RETURN_FAILED);
			//}
		//}
//
		//I2C_RawStop();
	//}
//
	//return(RETURN_OK);
//}


/*********************************************************************************/
RETURN_RESULT I2C_WriteByte(uint8_t chip_address, uint16_t reg_address, uint8_t data)
{
	if(I2C_BUS_NOT_BUSY)
	{
		if(I2C_RawStart(chip_address, I2C_WRITE))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawWrite(reg_address>>8))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawWrite(reg_address))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		if(I2C_RawWrite(data))
		{
			I2C_RawStop();
			return(RETURN_FAILED);
		}

		I2C_RawStop();

		return(RETURN_OK);
	}
	else
		return(RETURN_BUS_BUSY);
}


///*********************************************************************************/
//RETURN_RESULT I2C_WriteBytes(uint8_t chip_address, uint8_t reg_address, uint8_t data[], uint8_t size)
//{
	//if(I2C_BUS_NOT_BUSY)
	//{
		//uint8_t length = reg_address + size;
		//
		//for(; reg_address < length; reg_address++, data++)
		//{
			//if(I2C_WriteByte(chip_address, reg_address, *data))
			//{
				//return(RETURN_FAILED);
			//}
		//}
	//}
//
	//return(RETURN_OK);
//}


/*********************************************************************************/
void I2C_WriteBytes(uint8_t chip_address, uint8_t reg_address, uint8_t data[], uint8_t size)
{
	for(uint16_t idx=0; idx<size; idx++)
	{
		I2C_WriteByte(chip_address, reg_address+idx, data[idx]);
		delay_ms(3);
	}
}


/*********************************************************************************/
void I2C_ReadBytes(uint8_t address, uint8_t reg, uint8_t *data, uint8_t size)
{
	for(uint16_t idx=0; idx<size; idx++)
		I2C_ReadByte(address, reg+idx, data+idx);
}
