/*
 * @Author: yangyouguo
 * @Date: 2021-03-02 20:16:05
 * @LastEditors: yangyouguo
 * @LastEditTime: 2021-03-03 18:30:52
 * @FilePath: \FatFS\Inc\w25qxx.h
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W28QXX_H__
#define __W28QXX_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "spi.h"
#include "stdbool.h"
	
#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17


//////////////////////////////////////////////////////////////////////////////////

#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F

bool w25qxx_init(void);

uint8_t	 w25qxx_readSR(void);        		//¶ÁÈ¡×´Ì¬¼Ä´æÆ÷
void w25qxx_writeSR(uint8_t sr);  			//Ð´×´Ì¬¼Ä´æÆ÷
void w25qxx_write_enable(void);  		//Ð´Ê¹ÄÜ
void w25qxx_write_disable(void);		//Ð´±£»¤
void w25qxx_write_nocheck(uint8_t* pbuffer,uint32_t writeaddr,uint16_t numbytetowrite);
void w25qxx_read(uint8_t* pbuffer,uint32_t readaddr,uint16_t numbytetoread);   //¶ÁÈ¡flash
void w25qxx_write(uint8_t* pbuffer,uint32_t writeaddr,uint16_t numbytetowrite);//Ð´Èëflash
void w25qxx_erase_chip(void);    	  	//ÕûÆ¬²Á³ý
void w25qxx_erase_sector(uint32_t dst_addr);	//ÉÈÇø²Á³ý
void w25qxx_wait_busy(void);           	//µÈ´ý¿ÕÏÐ
void w25qxx_powerdown(void);        	//½øÈëµôµçÄ£Ê½
void w25qxx_wakeup(void);				//»½ÐÑ

#ifdef __cplusplus
}
#endif

#endif 

