/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "fatfs.h"

uint8_t retUSER;    /* Return value for USER */
char USERPath[4];   /* USER logical drive path */
FATFS USERFatFS;    /* File system object for USER logical drive */
FIL USERFile;       /* File object for USER */

/* USER CODE BEGIN Variables */
void mount_disk(void);
void format_disk(void);
void get_disk_info(void);
void create_file(void);
void read_file(void);
/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
	mount_disk();
	get_disk_info();
	create_file();
	read_file();
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */
void mount_disk(void)
{
  uint8_t res = f_mount(&USERFatFS, USERPath, 1);
  if (res == FR_NO_FILESYSTEM)
  {
		format_disk();
    //printf("FAILED: %d\n",res);
   return;
 }
  //printf("MOUNT OK\n");
}

void format_disk(void)
{
  uint8_t res = 0;
      
  //printf("PROCESSING...\n");
  res = f_mkfs(USERPath, 1, 4096);
  if (res == FR_OK)
  {
    HAL_Delay(100);
		return;
  }
  else
  {
		HAL_Delay(100);
    //printf("failed with: %d\n",res);
  }
}

void create_file(void)
{
  FIL file;
  FIL *pf = &file;
  uint8_t res;
	
  res = f_open(pf, "0:/test.txt", FA_OPEN_ALWAYS | FA_WRITE);
  if (res == FR_OK)
  {
    //printf("creat ok\n"); 				
  }
  else
  {
    //printf("creat failed\n");
   // printf("error code: %d\n",res);	
  }
  
  f_printf(pf, "hello fatfs!\n");
  
  res = f_close(pf);
    if (res != FR_OK)
    {
      //printf("close file error\n");
      //printf("error code: %d\n",res);				
    }
}

void get_disk_info(void)
{
	FATFS fs;
	FATFS *fls = &fs;
	FRESULT res;
	DWORD fre_clust;	
	uint16_t total_size=0;
	uint16_t free_size=0;
	res = f_getfree("/",&fre_clust,&fls);         /* Get Number of Free Clusters */
	if (res == FR_OK) 
	{
		total_size = ((fls->n_fatent-2)*fls->csize)*4;
		free_size = (fre_clust*fls->csize)*4;
	                                             /* Print free space in unit of MB (assuming 4096 bytes/sector) */
        //printf("%d KB Total Drive Space.\n"
         //      "%d KB Available Space.\n",
        //       ((fls->n_fatent-2)*fls->csize)*4,(fre_clust*fls->csize)*4);
	}
	else
	{
		//printf("get disk info error\n");
		//printf("error code: %d\n",res);
	}
	float per = free_size/total_size;
}

void read_file(void)
{
  FIL file;
  FRESULT res;
  UINT bw;
  uint8_t rbuf[100] = {0};
  
  res = f_open(&file, "0:/test.txt", FA_READ);
  if (res != FR_OK)
  {
    //printf("open error: %d\n",res);
    return;
  }
  f_read(&file, rbuf, 20, &bw);
  //printf("%s\n", rbuf);
  
  res = f_close(&file);
  if (res != FR_OK)
  {
     //printf("close file error\n");
     //printf("error code: %d\n",res);				
  }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
