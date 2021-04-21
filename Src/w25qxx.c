/*
 * @Author: yangyouguo
 * @Date: 2021-03-02 20:15:53
 * @LastEditors: yangyouguo
 * @LastEditTime: 2021-03-04 19:28:22
 * @FilePath: \FatFS\Src\w25qxx.c
 */
/* Includes ------------------------------------------------------------------*/
#include "w25qxx.h"
#include "string.h"
static uint8_t flash_test(void);
//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t w25qxx_readSR(void)
{
	uint8_t byte = 0;
	Spi1Select(Select); //使能器件
	Spi1WriteReadByte(W25X_ReadStatusReg); //发送读取状态寄存器命令
	byte = Spi1WriteReadByte(0Xff);		   //读取一个字节
	Spi1Select(UnSelect);
	HAL_Delay(1);
	return byte;
}
//写W25QXX状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void w25qxx_writeSR(uint8_t sr)
{
	Spi1Select(Select); //使能器件
	Spi1WriteReadByte(W25X_WriteStatusReg); //发送写取状态寄存器命令
	Spi1WriteReadByte(sr);					//写入一个字节
	Spi1Select(UnSelect);
	HAL_Delay(1);
}
//W25QXX写使能
//将WEL置位
void w25qxx_write_enable(void)
{
	Spi1Select(Select); //使能器件
	Spi1WriteReadByte(W25X_WriteEnable); //发送写使能
	Spi1Select(UnSelect);
	HAL_Delay(1);
}
//W25QXX写禁止
//将WEL清零
void w25qxx_write_disable(void)
{
	Spi1Select(Select); //使能器件
	Spi1WriteReadByte(W25X_WriteDisable); //发送写禁止指令
	Spi1Select(UnSelect);
	HAL_Delay(1);
}
//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
static uint16_t w25qxx_readID(void)
{
	uint16_t temp = 0;
	uint32_t read_id = W25X_ManufactDeviceID;
	uint16_t idle_dat = 0xffff;
	Spi1Select(Select);
	Spi1WriteReadDWord(read_id);
	temp = Spi1WriteReadWord(idle_dat);
	Spi1Select(UnSelect);
	return temp;
}

bool w25qxx_init()
{
	uint16_t chip_id = 0;
	uint8_t dat[500];

	if(w25qxx_readSR() == 0xfe)
	{
		w25qxx_writeSR(0);
		w25qxx_wait_busy();	  
	}
	
	chip_id = w25qxx_readID();
	if (chip_id == W25Q128)
	{
		return true;
	}
	return false;
}

//读取SPI FLASH
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//readaddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void w25qxx_read(uint8_t *pBuffer, uint32_t readaddr, uint16_t numbytetoread)
{
	uint16_t i = 0;
//	readaddr <<= 8;
//	readaddr |= W25X_ReadData;
	Spi1Select(Select); //使能器件

	//Spi1WriteDWord(readaddr);
	Spi1WriteReadByte(W25X_ReadData);
	Spi1WriteReadByte(readaddr >> 16);
	Spi1WriteReadByte(readaddr >> 8);
	Spi1WriteReadByte(readaddr);
	
	
	for (i = 0; i < numbytetoread; i++)
	{
		pBuffer[i] = Spi1WriteReadByte(0XFF); //循环读数
	}
	Spi1Select(UnSelect);
}
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//writeaddr:开始写入的地址(24bit)
//numbytetowrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
void w25qxx_write_page(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite)
{
	uint16_t i;
	w25qxx_write_enable(); //SET WEL
	Spi1Select(Select);	   //使能器件	
//	writeaddr <<= 8;
//	writeaddr |= W25X_PageProgram;
	//Spi1WriteDWord(writeaddr);
	Spi1WriteReadByte(W25X_PageProgram);
	Spi1WriteReadByte(writeaddr >> 16);
	Spi1WriteReadByte(writeaddr >> 8);
	Spi1WriteReadByte(writeaddr);
	
	for (i = 0; i < numbytetowrite; i++)
	{
		Spi1WriteReadByte(pbuffer[i]); //循环写数
	}
	
	Spi1Select(UnSelect); //取消片选
	w25qxx_wait_busy();	  //等待写入结束
}
//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//writeaddr:开始写入的地址(24bit)
//numbytetowrite:要写入的字节数(最大65535)
//CHECK OK
void w25qxx_write_nocheck(uint8_t *pbuffer, uint32_t writeaddr, uint16_t numbytetowrite)
{
	uint16_t pageremain;
	pageremain = 256 - writeaddr % 256; //单页剩余的字节数
	if (numbytetowrite <= pageremain)
		pageremain = numbytetowrite; //不大于256个字节
	while (1)
	{
		w25qxx_write_page(pbuffer, writeaddr, pageremain);
		if (numbytetowrite == pageremain)
			break; //写入结束了
		else	   //numbytetowrite>pageremain
		{
			pbuffer += pageremain;
			writeaddr += pageremain;

			numbytetowrite -= pageremain; //减去已经写入了的字节数
			if (numbytetowrite > 256)
				pageremain = 256; //一次可以写入256个字节
			else
				pageremain = numbytetowrite; //不够256个字节了
		}
	};
}
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//writeaddr:开始写入的地址(24bit)
//numbytetowrite:要写入的字节数(最大65535)
uint8_t w25qxx_buffer[4096];
void w25qxx_write(uint8_t *pBuffer, uint32_t writeaddr, uint16_t numbytetowrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;
	uint8_t *w25qxx_buf;
	w25qxx_buf = w25qxx_buffer;
	secpos = writeaddr / 4096; //扇区地址
	secoff = writeaddr % 4096; //在扇区内的偏移
	secremain = 4096 - secoff; //扇区剩余空间大小
	//printf("ad:%X,nb:%X\r\n",writeaddr,numbytetowrite);//测试用
	if (numbytetowrite <= secremain)
		secremain = numbytetowrite; //不大于4096个字节
	while (1)
	{
		w25qxx_read(w25qxx_buf, secpos * 4096, 4096); //读出整个扇区的内容
		for (i = 0; i < secremain; i++)				  //校验数据
		{
			if (w25qxx_buf[secoff + i] != 0XFF)
				break; //需要擦除
		}
		if (i < secremain) //需要擦除
		{
			w25qxx_erase_sector(secpos);	//擦除这个扇区
			for (i = 0; i < secremain; i++) //复制
			{
				w25qxx_buf[i + secoff] = pBuffer[i];
			}
			w25qxx_write_nocheck(w25qxx_buf, secpos * 4096, 4096); //写入整个扇区
		}
		else
			w25qxx_write_nocheck(pBuffer, writeaddr, secremain); //写已经擦除了的,直接写入扇区剩余区间.
		if (numbytetowrite == secremain)
			break; //写入结束了
		else	   //写入未结束
		{
			secpos++;	//扇区地址增1
			secoff = 0; //偏移位置为0

			pBuffer += secremain;		 //指针偏移
			writeaddr += secremain;		 //写地址偏移
			numbytetowrite -= secremain; //字节数递减
			if (numbytetowrite > 4096)
				secremain = 4096; //下一个扇区还是写不完
			else
				secremain = numbytetowrite; //下一个扇区可以写完了
		}
	};
}
//擦除整个芯片
//等待时间超长...
void w25qxx_erase_chip(void)
{
	w25qxx_write_enable(); //SET WEL
	w25qxx_wait_busy();
	Spi1Select(Select); //使能器件	
	Spi1WriteByte(W25X_ChipErase);
	//Spi1WriteReadByte(W25X_ChipErase); //发送片擦除命令
	Spi1Select(UnSelect); //取消片选
	w25qxx_wait_busy();	  //等待芯片擦除结束

}
//擦除一个扇区
//dst_addr:扇区地址 根据实际容量设置
//擦除一个山区的最少时间:150ms
void w25qxx_erase_sector(uint32_t dst_addr)
{
	dst_addr *= 4096;
	w25qxx_write_enable(); //SET WEL
	w25qxx_wait_busy();
	Spi1Select(Select);	   //使能器件
	
	//Spi1WriteByte(W25X_SectorErase);
	//dst_addr <<= 8;
	//dst_addr |= W25X_SectorErase;
	//	dst_addr &= 0xffffff;
	//	dst_addr |= W25X_SectorErase << 24;
	//	Spi1WriteReadDWord(dst_addr);
	//Spi1WriteDWord(dst_addr);
		Spi1WriteReadDWord(W25X_SectorErase);			 //发送扇区擦除指令
		Spi1WriteReadDWord((uint8_t)((dst_addr) >> 16)); //发送24bit地址
		Spi1WriteReadDWord((uint8_t)((dst_addr) >> 8));
		Spi1WriteReadDWord((uint8_t)dst_addr);
	
	Spi1Select(UnSelect); //取消片选
	w25qxx_wait_busy();	  //等待擦除完成
}
//等待空闲
void w25qxx_wait_busy(void)
{
	while ((w25qxx_readSR() & 0x01) == 0x01)
		; // 等待BUSY位清空
}
//进入掉电模式
void w25qxx_powerdown(void)
{
	Spi1Select(Select);				   //使能器件
	Spi1WriteReadByte(W25X_PowerDown); //发送掉电命令
	Spi1Select(UnSelect);			   //取消片选
	HAL_Delay(1);					   //等待TPD
}
//唤醒
void w25qxx_wakeup(void)
{
	Spi1Select(Select);						  //使能器件
	Spi1WriteReadByte(W25X_ReleasePowerDown); //  send W25X_PowerDown command 0xAB
	Spi1Select(UnSelect);					  //取消片选
	HAL_Delay(1);							  //等待TRES1
}

__attribute__((unused)) static uint8_t flash_test()
{
	uint16_t i = 0;
	uint8_t dat[5];

	for (i = 0; i < sizeof(dat); i++)
	{
		dat[i] = i;
	}

	w25qxx_write(dat, 4096, sizeof(dat));

	memset(dat, 0, sizeof(dat));

	w25qxx_read(dat, 4096, sizeof(dat));
	for (i = 0; i < sizeof(dat); i++)
	{
		if (dat[i] != (uint8_t)i)
			return 1;
	}

	return 0;
}
