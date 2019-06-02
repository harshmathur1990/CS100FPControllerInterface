// SNIPPET TO TEST THE PROGS WITH DELL 5810
// Written on 14.05.2018
// Shibu K. Mathew 

#include <iostream>
#include <cstddef>
#include <cstring>
#include <stdio.h>
#include <cstdlib>
#include <windows.h>
#include <stdlib.h>
#include <cstring>
#include <ctime>
#include <time.h>
#include <math.h>
#include <conio.h>
#include <dir.h>
#include <string>

// Sensicam specific header files
#include "PCO_err.h"
#include "sc2_SDKStructures.h"
#include "SC2_SDKAddendum.h"
#include "SC2_CamExport.h"
#include "sc2_defs.h"

//Fits I/O specific header files
#include "fitsio.h"
#include "longnam.h"
//#include "FITSIO_INCLUDE/CAIIFEI.h"
#include "pco_camera.h"


int write_to_fits(char *filename, int width, int height, void *data, int expt);


int capture_this(char *filename, int exposure_time)
{
	int iRet;
	HANDLE cam,hEvent;
	WORD* data;
//	int exposure_time=10;
	short bufferNr = -1;
	WORD actsizex, actsizey, ccdsizex, ccdsizey;
	PCO_Description strDescription;
	iRet = PCO_OpenCamera(&cam, 0);
	printf("%d\n",iRet);
	strDescription.wSize=sizeof(PCO_Description);
    iRet = PCO_GetCameraDescription(cam,&strDescription);\
    printf("PCO_GetCameraDescription: %d \n", iRet);
    WORD ms = 0x0002;
    iRet = PCO_SetDelayExposureTime(cam, 0, exposure_time, ms, ms);
    printf("PCO_SetDelayExposureTime: %d \n", iRet);
    iRet = PCO_ArmCamera(cam);
    printf("PCO_ArmCamera: %d \n", iRet);
    iRet=PCO_GetSizes(cam, &actsizex, &actsizey, &ccdsizex, &ccdsizey);
    printf("Sizes %d - %d - %d - %d \n",actsizex, actsizey, ccdsizex, ccdsizey);
    iRet=PCO_AllocateBuffer(cam, &bufferNr, actsizex * actsizey * sizeof(WORD), &data, &hEvent);
    printf("PCO_AllocateBuffer: %d \n", iRet);
//    size = actsizex *actsizey*2;
//	frame = (unsigned short *)VirtualAlloc(NULL, actsizex *actsizey*2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//    iRet = PCO_SetImageParameters(cam,actsizex,actsizey,IMAGEPARAMETERS_READ_WHILE_RECORDING,NULL,0);
	iRet=PCO_SetRecordingState(cam, 0x0001);
	printf("PCO_SetRecordingState: %d \n", iRet);
//    iRet=PCO_AddBufferEx(cam, 0, 0, bufferNr, actsizex, actsizey, 16);
//    printf("PCO_AddBufferEx: %d \n", iRet);
    iRet = PCO_GetImageEx(cam, 1, 0, 0, bufferNr, actsizex, actsizey, 16);
    if (iRet != PCO_NOERROR)
	{
		printf("failed: %d \n", iRet);
	}
	else
		printf("done ");
	
	write_to_fits(filename, actsizex, actsizey, data, exposure_time);
	iRet = PCO_SetRecordingState(cam, 0);
	iRet = PCO_FreeBuffer(cam, bufferNr);
	iRet = PCO_CloseCamera(cam);

}



int write_to_fits(char *filename, int width, int height, void *data, int expt)
{
  int status = 0;
  long naxes[2] = {width, height};
  fitsfile *fp;
  
  fits_create_file(&fp, filename, &status);
  if (status)
  {
		printf("fits_create_file: %d\n", status);
		fits_report_error(stderr, status);
    return -1;
  }

  fits_create_img(fp, USHORT_IMG, 2, naxes, &status);
  if (status)
  {
		printf("fits_create_img: %d\n", status);
    fits_report_error(stderr, status);
    return -1;
  }

	fits_write_img(fp, TUSHORT, 1, (width * height), data, &status);
	fits_update_key(fp,TLONG,"EXPOSURE",&expt,"Total Exposure time in ms",&status);
//    fits_update_key(fp,TSTRING,"TIME    ",time,"Date and time",&status);
//    fits_update_key(fp,TSTRING,"FILTER  ",filname,"Filter",&status);
//    fits_update_key(fp,TDOUBLE,"CAM_TEMP",&outt,"Camera temperature",&status);
  if (status)
  {
		printf("fits_write_img: %d\n", status);
		fits_report_error(stderr, status);
    return -1;
  }

  fits_close_file(fp, &status);
  if (status)
  {
		printf("fits_close_file: %d\n", status);
    fits_report_error(stderr, status);
    return -1;
  }

  return 0;
}

