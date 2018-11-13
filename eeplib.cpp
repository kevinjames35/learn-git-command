#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/time.h>
#include "eeplib.h"

#define SLOT_SWITCHL2	(0x72 << 1)

uint16_t wEepDeviceExist =0;
uint16_t SMBus_Base_Addr=0;

SMBUS_IO smbusio = {0,0,0,0,0,0,0,0};

uint16_t wDebugLevel = 0;
void (*Lvl3DebugCB)(int8_t* strFunName, void* param1, void* param2, void* param3, int8_t type) = NULL;

void __init_smbusio(uint16_t uwSmbusBaseAddr);
bool _raw_switch_slot(uint8_t ubSlot);
bool _raw_switch_slotL2(uint8_t ubSlot);
bool _raw_read_smbus(SMBUS_ACCESS *devcmd);
bool _raw_write_smbus(SMBUS_ACCESS *devcmd);
1000;
		if ( (x2-x1) >= ms ) fEnding = true;
	}	
}

/*****************************************************************************/
/*****  Chapter 3.7         Serial EEPROM                                *****/
/*****************************************************************************/
#if defined(EEP_USES_SMBUS)
	#include "eeplib_std.cpp"
#endif
#if defined(EEP_USES_SMBUS_PLATFORM)
	#include "eeplib_platform.cpp"
#endif



/******************************************************************/
/******************   inner fucntion call *************************/
/******************************************************************/
/*****************/
bool _raw_switch_slot(uint8_t ubSlot)
{
SMBUS_ACCESS swdev;

	if ( ubSlot != 0 )	swdev.cmd = 0x01 << (ubSlot-1);
	else 			swdev.cmd = 0;
	swdev.addr = SLOT_SWITCH;
	swdev.ctrl = SMB_ByteAccess;
	if (_raw_read_smbus(&swdev)) return true;
	return false;	
}

/*****************/
bool _raw_switch_slotL2(uint8_t ubSlot)
{
SMBUS_ACCESS swdev;

	if ( ubSlot != 0 )	swdev.cmd = 0x01 << (ubSlot-1);
	else 			swdev.cmd = 0;
	swdev.addr = SLOT_SWITCHL2;
	swdev.ctrl = SMB_ByteAccess;
	if (_raw_read_smbus(&swdev)) return true;
	return false;	
}

/*** Byte Read ***/
bool _raw_read_smbus(SMBUS_ACCESS *devcmd)
{
bool bRet=false;
uint8_t xi, xlen;

	outb(devcmd->cmd, smbusio.Cmd);
	outb(devcmd->addr | 0x01, smbusio.Addr);
	if ( devcmd->ctrl == SMB_BlockAccess ) {
		outb(devcmd->arData[0], smbusio.Data0 ); //bloack read length
		outb(0x02, smbusio.AuxCtrl);	//enable E32B		
	}
	outb(devcmd->ctrl, smbusio.Ctrl);
	if ( _check_smbus_busy(&smbusio) ) {
		outb(0xFF, smbusio.Status);    //clear DEV_ERR status and interrupt status
		return bRet;	//wait finish	
	}
	switch (devcmd->ctrl) {	
	case SMB_ByteAccess:
		devcmd->arData[0] = inb(smbusio.Data0);
		break;
	case SMB_WordAccess:
		*(uint16_t*)devcmd->arData = inw(smbusio.Data0);
 		break;
	case SMB_BlockAccess:
		inb(smbusio.Cmd); //clear block pointer
		xlen = inb(smbusio.Data0);
		for ( xi=0 ; xi<xlen ; xi++ ) {
			devcmd->arData[xi] = inb(smbusio.BlockDB);
		}
		devcmd->arData[xlen]=0x00;
		outb(0x00, smbusio.AuxCtrl);	//disable E32B	
		break;
	default:
		//error handle ???
		break;
	}
        outb(2, smbusio.Status);    //clear interrupt status
 	bRet = true;    
	return bRet;
}
/*************************/
/*** write smbus data ****/
/*************************/
bool _raw_write_smbus(SMBUS_ACCESS *devcmd)
{
bool bRet=false;
	
        outb(devcmd->cmd, smbusio.Cmd);
        outb(devcmd->addr, smbusio.Addr);
	switch (devcmd->ctrl) {	
	case SMB_ByteAccess:
		outb(devcmd->arData[0], smbusio.Data0);
		break;
	case SMB_WordAccess:
		outb(devcmd->arData[0], smbusio.Data0);
		outb(devcmd->arData[1], smbusio.Data1);
		break;
	case SMB_BlockAccess:
		//???????????????????? TBD
		break;
	default:
		//error handle ???
		break;
	}
        outb(devcmd->ctrl, smbusio.Ctrl);
	if ( _check_smbus_busy(&smbusio) ) {
		outb(0xFF, smbusio.Status);    //clear DEV_ERR status and interrupt status
		return bRet;	//wait finish	
	}
        outb(2, smbusio.Status);    //clear interrupt status
	bRet=true;
	return bRet;
}

/***********************/
bool _check_smbus_busy(SMBUS_IO *smbusio)
{
uint8_t xch;
int	xi;
	for (xi=0; xi < 0x800 ; xi++) {
		xch=inb(smbusio->Status);
		xch &= 0x8f ;
		if 	( xch == 0x80 ) outb(0x80, smbusio->Status);
		else if ( xch == 0x04 ) {
			outb(0x40, smbusio->Status);
			return true;
		}
		else if ( xch == 0x02 ) outb(0x02, smbusio->Status);
		else if ( xch == 0x00 ) {
			return false;
		}
		_xmsdelay(1);
	}
	return true; 


}
/*************************/
void xdelay(int loop)
{
int	xxi,xxj,xxk=0,xloop;
	for (xloop=0 ; xloop<loop ; xloop++) {
		for (xxi=0 ; xxi< 0x1000 ; xxi++) {
			for (xxj=0 ; xxj < 0x30 ; xxj++) {
				xxk++;
			}
		}
	}
}
/***************************/
void __init_smbusio(uint16_t uwSmbusBaseAddr)
{
	SMBus_Base_Addr = uwSmbusBaseAddr;
	smbusio.Cmd     = SMBus_Base_Addr +3;
	smbusio.Addr    = SMBus_Base_Addr +4;
	smbusio.Data0   = SMBus_Base_Addr +5;
	smbusio.Data1   = SMBus_Base_Addr +6;
	smbusio.Ctrl    = SMBus_Base_Addr +2;
	smbusio.Status  = SMBus_Base_Addr +0;
	smbusio.BlockDB = SMBus_Base_Addr +7;
	smbusio.AuxCtrl = SMBus_Base_Addr +0xd;
}


/****************************************************************************/
int32_t _brdHookDebugCallback(DEBUG_CBDATA pDebugCB, uint16_t uwLevel)
{
#if SDK_DEBUG_ENABLE
	wDebugLevel = uwLevel;
	Lvl3DebugCB = pDebugCB;	
#endif
	return ERR_Success;
}
