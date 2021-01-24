#include "def.h"


/*********************************************************************************/
void Init_Memory(void)
{
	uint8_t data_to_write[4];
	
	
	PWR_MEM_ON
	
	// unlock I2C operations
	I2C_WriteByte(SYSTEM_SPACE_ADDRESS, SYSTEM_I2C_LOCK, 0);
		
	
	// configure Energy Harvesting Mode
	I2C_WriteByte(SYSTEM_SPACE_ADDRESS, SYSTEM_CFG_REG, 0x03);
			
	// clear SSS fields
	for(uint8_t idx=0; idx<4; idx++)
		data_to_write[idx] = 0;
		
	I2C_WriteBytes(SYSTEM_SPACE_ADDRESS, SYSTEM_SSS_REG, data_to_write, 4);
	
	// write configuration
	I2C_WriteBytes(USER_SPACE_ADDRESS, CFG_ADDRESS, (uint8_t*)(&tlog_cfg), 4);
	
	PWR_MEM_OFF
}