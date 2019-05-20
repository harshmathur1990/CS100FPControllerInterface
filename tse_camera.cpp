// Camera control program for Finger Lake Instruments, PL 16803
// Written by Shibu K. Mathew, 09 July 2009
// Later on this will be integrated with IDL Image capture routine

// Essential include files
	#include "stdafx.h"
#include "XGetopt.h"
//#include <unistd.h>
#include "fitsio.h"
#include "libfli.h"
#include <cstring>
#include <ctime>
#include <time.h>
#include "tse_camera.h"

void delay(double);

#define TRYFUNC(f, a)		    \
  do {						    \
    if ((r = f a))			    \
      printf(#f "() failed\n");	\
  } while (0)

// Images will be written in FITS format 

int writefits(char *filename, int width, int height, void *data, int expt, char *time, char *filname, double outt);

typedef struct _cam_t{
        flidomain_t domain;
        char *dname;
        char *name;
        } cam_t;
        
int capture (char *filname, int exptime, char *name)
{



flidomain_t domain;
flidev_t dev;
double temperature =-20.0,d1,d2,outtemp;
long tmp1,tmp2,tmp3,tmp4,r,img_size;
unsigned short *img;
int hbin=1,vbin=1,flushes=1,row_width,img_rows,row,tdi_rate=0,yn,i=0,j=0,pics=6;  //binning hbin=2,vbin=2

char *fname="MER";
char ** names;
char time_f[80];
size_t si;
#define BUFF_SIZ 4096
char buff[BUFF_SIZ];
time_t rawtime;
struct tm *syst;
char fn[80];
SYSTEMTIME ptime;


        

   // int exptime; // [6]={25,50,75,100,125,150};
   //char * filname;
     int jj=0;
      //for (i=0;i<100;i++)
      //{
      yn = FLIList(FLIDOMAIN_USB|FLIDEVICE_CAMERA,&names);
      if (names == NULL )
      {
             printf("No cameras found, sorry --> Exiting\n");
             exit (1);
      }
     
      if (( yn=FLIOpen(&dev, "flipro0",FLIDEVICE_CAMERA|FLIDOMAIN_USB)))
      {
      printf("Problem in opening camera --> %s\n",names[0]);
      exit (1);
      }	 
      if ((yn = FLISetTemperature(dev, temperature)))
      {
      printf("Problem in setting temperature to --> %f degC, check the camera\n", temperature);
      }
      
      if (( yn= FLIGetPixelSize (dev, &d1, &d2)))
      {
      printf("Problem in getting the pixel size\n");
      } 
           
      if (( yn = FLIGetArrayArea (dev, &tmp1, &tmp2, &tmp3, &tmp4)))
      {
      printf("Problem in getting the array area\n");
      }
      
      if (( yn = FLIGetVisibleArea (dev, &tmp1, &tmp2, &tmp3, &tmp4)))
      {
      printf("Problem in getting the visible area\n");
      }
      
      if ((yn = FLISetImageArea(dev, tmp1, tmp2, tmp1 + (tmp3 - tmp1) / hbin, tmp2 + (tmp4 - tmp2) / vbin)))
      {
      printf("Problem in setting the image area \n");
      }
      
      if ((yn = FLISetNFlushes(dev, flushes)))
      {
      printf("Problem in setting up the CCD flushese\n");
      }
      
      if ((yn=FLISetExposureTime(dev, exptime)))
      {
      printf("Problem in setting exposure time\n");
      }
      
       if ((yn = FLISetFrameType(dev, FLI_FRAME_TYPE_NORMAL)))
      {
               printf("Problem in setting the frame type \n");
      } 
      
      if (( yn = FLISetHBin (dev, hbin)))
      {
      printf("Problem in setting up H-bin \n"); 
      }
      
      if ((yn = FLISetVBin (dev, vbin)))
      {
      printf("Problem in setting up the V-bin \n"); 
      }
      
      row_width = (tmp3 - tmp1) / hbin;
      img_rows = (tmp4 - tmp2) / vbin; 
      
      img_size = img_rows * row_width * sizeof(unsigned short);
      if ((img = (unsigned short *) malloc(img_size)) == NULL)
      {
      printf("malloc() failed\n");
      exit(1);
      }
      
      time(&rawtime);
      syst=localtime(&rawtime);
      strftime(time_f,18,"%x;%X",syst);
      strftime(fn,18,"%Y%m%d%H%M%S",syst);
      
      if ((yn = FLIGetTemperature(dev, &outtemp)))
      {
      printf("Problem in reading the temperature\n");
      }
      
      TRYFUNC(FLIExposeFrame, (dev));
      do {
	  TRYFUNC(FLIGetExposureStatus, (dev, &tmp1));
	  if (r)
      break;
	  Sleep(tmp1);
      } while (tmp1);

      for (row = 0; row < img_rows; row++)
      {
	TRYFUNC(FLIGrabRow, (dev, &img[row * row_width], row_width));
	if (r)
    break;
      } 
 #define WRITEIMG(writefn, ext)						                \
  do {									                            \
  GetLocalTime(&ptime);	\
  if (_snprintf(buff, BUFF_SIZ,"MR.%02d%02d%04d.%02d%02d%02d.%3d.STEP_%s." ext,ptime.wDay,ptime.wMonth,ptime.wYear,ptime.wHour,ptime.wMinute,ptime.wSecond,ptime.wMilliseconds, name) == -1) \
      printf("output file name too long\n");                        \
    TRYFUNC(writefn, (buff, row_width, img_rows, img,exptime,time_f,filname,outtemp));	    \
  } while (0)
      WRITEIMG(writefits, "fits");
#undef WRITEIMG

yn =FLIClose(dev);

}

//Function image in FITS format

int writefits(char *filename, int width, int height, void *data, int expt, char *time, char *filname, double outt)
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
    fits_update_key(fp,TSTRING,"TIME    ",time,"Date and time",&status);
    fits_update_key(fp,TSTRING,"FILTER  ",filname,"Filter",&status);
    fits_update_key(fp,TDOUBLE,"CAM_TEMP",&outt,"Camera temperature",&status);
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
      
void delay(double seconds)
{
LARGE_INTEGER timestart,timeend,systemfrequency;
double millisecondsleft;
QueryPerformanceFrequency(&systemfrequency);
QueryPerformanceCounter(&timestart);
QueryPerformanceCounter(&timeend);
__int64 timetaken= (timeend.QuadPart-timestart.QuadPart);
millisecondsleft=(timetaken)*1000./systemfrequency.QuadPart;
while (millisecondsleft < (double) seconds*1e3 )
{
QueryPerformanceCounter(&timeend);
__int64 timetaken= (timeend.QuadPart-timestart.QuadPart);
millisecondsleft=(timetaken)*1000./systemfrequency.QuadPart;
}
}  
   

