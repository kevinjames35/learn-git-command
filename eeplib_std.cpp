/*
 * eeplib_std.cpp
 *
 *  Created on: Aug 27, 2018
 *      Author: legend
 */

/*****************************************************************************/
/*****  Chapter 3.7         Serial EEPROM                                *****/
/*****************************************************************************/
//**********
//*** 3.7.1
int32_t _brdLMB_EEP_QueryDevices(uint16_t* puwSlotsDev, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
SMBUS_ACCESS i2ccmd;
int xi;	
int32_t iResult = ERR_Success;
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_QueryDevices ) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_QueryDevices",  NULL, NULL, NULL, DEBUG_LEVEL3_INPUT);
	}
	#endif	
	if ( SMBus_Base_Addr == 0 ) __init_smbusio(uwSmbusBaseAddr);
	//mutex lock
	ioperm(SMBus_Base_Addr, 0x20, 1);
	*puwSlotsDev = 0;

	for ( xi=0 ; xi<=maxslots ; xi++) {
		if ( xi == SLOT_ONBOARD ) {
			i2ccmd.addr = ONBOARD_EEPROM;
		}
		else {		
			_raw_switch_slot(xi);
			i2ccmd.addr = SLOT_EEPROM;
		}
		i2ccmd.ctrl = SMB_ByteAccess;
		i2ccmd.cmd = 0; //address 0
		if ( _raw_read_smbus(&i2ccmd) ) *puwSlotsDev |= (0x01 << xi);			
	}

	wEepDeviceExist = *puwSlotsDev ;
	//mutex unlock
	ioperm(SMBus_Base_Addr, 0x20, 0);
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_QueryDevices) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_QueryDevices", (void*)&iResult, (void*)puwSlotsDev, NULL, DEBUG_LEVEL3_OUTPUT);
	}
	#endif
	return iResult;
}
//**********
//*** 3.7.2
int32_t _brdLMB_EEP_WriteByte(uint8_t ubDeviceNo, uint32_t udwAddr, uint8_t ubData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_WriteBlock(ubDeviceNo, udwAddr, sizeof(ubData), (uint8_t*)&ubData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.3
int32_t _brdLMB_EEP_WriteWord(uint8_t ubDeviceNo, uint32_t udwAddr, uint16_t uwData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_WriteBlock(ubDeviceNo, udwAddr, sizeof(uwData), (uint8_t*)&uwData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.4
int32_t _brdLMB_EEP_WriteDWord(uint8_t ubDeviceNo, uint32_t udwAddr, uint32_t udwData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_WriteBlock(ubDeviceNo, udwAddr, sizeof(udwData), (uint8_t*)&udwData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.5
int32_t _brdLMB_EEP_WriteBlock(uint8_t ubDeviceNo, uint32_t udwAddr, uint16_t uwLength, uint8_t* pubBlock, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
SMBUS_ACCESS i2ccmd;
int32_t iResult = ERR_Success;
uint16_t xi;
uint16_t wData;
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_WriteBlock ) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_WriteBlock",  (void*)&ubDeviceNo, (void*)&udwAddr, (void*)&uwLength, DEBUG_LEVEL3_INPUT);
	}
	#endif	
	ioperm(SMBus_Base_Addr, 0x20, 1);
	try {		
		if ( ubDeviceNo > maxslots ) throw (int32_t)ERR_NotSupport;
		if ( wEepDeviceExist == 0 ) {
			if ( _brdLMB_EEP_QueryDevices(&wData, uwSmbusBaseAddr, maxslots) != ERR_Success ) throw (int32_t)ERR_NotSupport;
			ioperm(SMBus_Base_Addr, 0x20, 1);
		}		
		wData = 0x01 << ubDeviceNo ;
		if ( !(wData & wEepDeviceExist) ) throw (int32_t)ERR_NotExist; 
		if ( udwAddr > EEPROM_MAXSIZE ) throw (int32_t)ERR_Invalid;
		if ( (udwAddr + uwLength ) > EEPROM_MAXSIZE ) throw (int32_t)ERR_Invalid;
		
		if ( ubDeviceNo == SLOT_ONBOARD ) {
			i2ccmd.addr = ONBOARD_EEPROM;
		}
		else {		
			_raw_switch_slot(ubDeviceNo);
			i2ccmd.addr = SLOT_EEPROM;
		}
	
		for ( xi=0 ; xi<uwLength ; xi++ ) {
			i2ccmd.ctrl = SMB_ByteAccess;
			i2ccmd.cmd = (uint8_t)(udwAddr+xi);
			i2ccmd.arData[0] = *pubBlock++;
			if ( !_raw_write_smbus(&i2ccmd) ) { 
				//write 2nd
				_xmsdelay(2);
				if ( !_raw_write_smbus(&i2ccmd) ) {
					//write 3th
					_xmsdelay(2);
					if ( !_raw_write_smbus(&i2ccmd) ) {
						iResult = ERR_Error;		
						xi = uwLength + 100;
					}
	
				}
	
			}	
			_xmsdelay(2);
		}
	}
	catch (int32_t err) {
		iResult = err;
	}
	ioperm(SMBus_Base_Addr, 0x20, 0);
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_WriteBlock) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_WriteBlock", (void*)&iResult, NULL, NULL, DEBUG_LEVEL3_OUTPUT);
	}
	#endif	
	return iResult;	
}
//**********
//*** 3.7.6
int32_t _brdLMB_EEP_ReadByte(uint8_t ubDeviceNo, uint32_t udwAddr, uint8_t* pubData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_ReadBlock(ubDeviceNo, udwAddr, sizeof(uint8_t), pubData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.7
int32_t _brdLMB_EEP_ReadWord(uint8_t ubDeviceNo, uint32_t udwAddr, uint16_t* puwData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_ReadBlock(ubDeviceNo, udwAddr, sizeof(uint16_t), (uint8_t*)puwData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.8
int32_t _brdLMB_EEP_ReadDWord(uint8_t ubDeviceNo, uint32_t udwAddr, uint32_t* pudwData, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
	return _brdLMB_EEP_ReadBlock(ubDeviceNo, udwAddr, sizeof(uint32_t), (uint8_t*)pudwData, uwSmbusBaseAddr, maxslots);
}
//**********
//*** 3.7.9
int32_t _brdLMB_EEP_ReadBlock(uint8_t ubDeviceNo, uint32_t udwAddr, uint16_t uwLength, uint8_t* pubBlock, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
SMBUS_ACCESS i2ccmd;
int32_t iResult = ERR_Success;
uint16_t xi;
uint16_t wData;
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_ReadBlock) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_ReadBlock",  (void*)&ubDeviceNo, (void*)&udwAddr, (void*)&uwLength, DEBUG_LEVEL3_INPUT);
	}
	#endif		
	ioperm(SMBus_Base_Addr, 0x20, 1);
	try {
		if ( ubDeviceNo > maxslots ) throw (int32_t)ERR_NotSupport;
		if ( wEepDeviceExist == 0 ) {
			if ( _brdLMB_EEP_QueryDevices(&wData, uwSmbusBaseAddr, maxslots) != ERR_Success ) throw (int32_t)ERR_NotSupport;
			ioperm(SMBus_Base_Addr, 0x20, 1);
		}
		wData = 0x01 << ubDeviceNo ;
		if ( !(wData & wEepDeviceExist) ) throw (int32_t)ERR_NotExist; 
		if ( udwAddr > EEPROM_MAXSIZE ) throw (int32_t)ERR_Invalid;
		if ( (udwAddr + uwLength ) > EEPROM_MAXSIZE ) throw (int32_t)ERR_Invalid;
		
		if ( ubDeviceNo == SLOT_ONBOARD ) {
			i2ccmd.addr = ONBOARD_EEPROM;
		}
		else {		
			_raw_switch_slot(ubDeviceNo);
			i2ccmd.addr = SLOT_EEPROM;
		}

		for ( xi=0 ; xi<uwLength ; xi++ ) {
			i2ccmd.ctrl = SMB_ByteAccess;
			i2ccmd.cmd = (uint8_t)(udwAddr+xi);
			if ( _raw_read_smbus(&i2ccmd) ) { 
				*pubBlock++ = i2ccmd.arData[0];
			}
			else {
				//read 2nd
				if ( _raw_read_smbus(&i2ccmd) ) {
					*pubBlock++ = i2ccmd.arData[0];
				}
				else {
					//read 3th
					if ( _raw_read_smbus(&i2ccmd) ) {
						*pubBlock++ = i2ccmd.arData[0];
					} 			
					else {
						iResult = ERR_Error;	
						xi = uwLength + 100;
					}
				}
			}
			//_xmsdelay(1);	
		}
	}
	catch (int32_t err) {
		iResult=err;
	}
	ioperm(SMBus_Base_Addr, 0x20, 0);
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_ReadBlock) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_ReadBlock", (void*)&iResult, NULL, NULL, DEBUG_LEVEL3_OUTPUT);
	}
	#endif
	return iResult;	
}
//**********
//*** 3.7.10
int32_t _brdLMB_EEP_Erase(uint8_t ubDeviceNo, uint16_t uwSmbusBaseAddr, uint16_t maxslots)
{
SMBUS_ACCESS i2ccmd;
int32_t iResult = ERR_Success;
int xi;
uint16_t wData;
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_Erase) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_Erase",  (void*)&ubDeviceNo, NULL, NULL, DEBUG_LEVEL3_INPUT);
	}
	#endif	
	ioperm(SMBus_Base_Addr, 0x20, 1);
	try {
		if ( ubDeviceNo > maxslots ) throw (int32_t)ERR_NotSupport;
		if ( wEepDeviceExist == 0 ) {
			if ( _brdLMB_EEP_QueryDevices(&wData, uwSmbusBaseAddr, maxslots) != ERR_Success ) throw (int32_t)ERR_NotSupport;
			ioperm(SMBus_Base_Addr, 0x20, 1);
		}	
		wData = 0x01 << ubDeviceNo ;
		if ( !(wData & wEepDeviceExist) ) throw (int32_t)ERR_NotExist; 

		if ( ubDeviceNo == SLOT_ONBOARD ) {
			i2ccmd.addr = ONBOARD_EEPROM;
		}
		else {		
			_raw_switch_slot(ubDeviceNo);
			i2ccmd.addr = SLOT_EEPROM;
		}
	
		for ( xi=0 ; xi<EEPROM_MAXSIZE ; xi++ ) {
			i2ccmd.ctrl = SMB_ByteAccess;
			i2ccmd.cmd = (uint8_t)xi;
			i2ccmd.arData[0] = 0;
			if ( !_raw_write_smbus(&i2ccmd) ) { 
				//write 2nd
				_xmsdelay(2);
				if ( !_raw_write_smbus(&i2ccmd) ) {
					//write 3th
					_xmsdelay(2);
					if ( !_raw_write_smbus(&i2ccmd) ) {
						iResult = ERR_Error;		
						xi = EEPROM_MAXSIZE + 100;
					}
				}
			}	
			_xmsdelay(2);
		}	
	}
	catch (int32_t err) {
		iResult = err;
	}
	ioperm(SMBus_Base_Addr, 0x20, 0);
	#if SDK_DEBUG_ENABLE
	if ( wDebugLevel == LOGGING_ALL || wDebugLevel == LOGGING_EEP || wDebugLevel == LOG_LMB_EEP_Erase) {
		if ( Lvl3DebugCB != NULL ) Lvl3DebugCB((int8_t*)"LMB_EEP_Erase", (void*)&iResult, NULL, NULL, DEBUG_LEVEL3_OUTPUT);
	}
	#endif
	return iResult;	
}	




