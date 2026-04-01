/********************************************************************************
** ÎÄ¼þÃû³Æ: GPS.h
** Ãè    Êö: 
** ´´½¨×÷Õß: ÉîÛÚÊÐºÍ³É¹¤¿Ø¿Æ¼¼ÓÐÏÞ¹«Ë¾
** ½¨Á¢ÈÕÆÚ: 
** °æ±¾±ä¸ü: 
********************************************************************************/
#ifndef __GPS_H
#define __GPS_H

/**********************************Í·ÎÄ¼þ°üº¬***********************************/
#include "common.h"
/************************************ºê¶¨Òå*************************************/

#define check_len 100
#define GPS_rx_len 1000

/*********************************Êý¾ÝÀàÐÍ¶¨Òå**********************************/
 //½âÎöGPS¶¨Î»ÐÅºÅ
// typedef struct 
// {
// 	unsigned short int year;
// 	unsigned char month;
// 	unsigned char day;
// 	unsigned char hour;
// 	unsigned char Minute;
// 	unsigned char second;

// 	double longitude;
// 	unsigned char longitude_suffix; // E»òW

// 	double latitude;
// 	unsigned char latitude_suffix;
// 	unsigned char position_valid; // ÓÐÐ§
	
// 	double height;		//º£°Î
	
// 	double speed;			//ËÙ¶È
	
// 	double angle;			//º½Ïò½Ç
	
// 	char longitudeBuf[50];
// 	char latitudeBuf[50];
	
// }GPS_InfoDef;

// typedef struct 
// {
// 	unsigned int year;
// 	unsigned char month;
// 	unsigned char day;
// 	unsigned char hour;
// 	unsigned char Minute;
// 	unsigned char second;
// }China_TimeDef;

// extern China_TimeDef China_Time;

// typedef struct
// {
// 	uint8_t longitude[50];	//¾­¶È
// 	uint8_t latitude[50];		//Î¬¶È
// 	uint8_t height[50];			//º£°Î
// 	double  speed;
// 	double  angle;
// 	uint8_t have_get_data_flag;
// 	uint8_t flag;
	
// }GPS_Data_t;

typedef struct
{
	u8 UTC_hour;
	u8 UTC_min;
	u8 UTC_sec;
	u16 UTC_msec;
	char acitve;//¶¨Î»ÓÐÐ§ÐÔ VÎÞÐ§ AÓÐÐ§

	char latitude[20];	//Î¬¶È
	char latitude_dir;	//Î¬¶È·½Ïò 	N±±Î³ S ÄÏÎ³
	char longitude[20];	//¾­¶È
	char longitude_dir;//¾­¶È·½Ïò E¶«¾­ W Î÷¾­ 
	// float speed;		//¶ÔµØËÙ¶È
	u16 speed;
	u16 angle;		//¶ÔµØº½Ïò½Ç
	u8 day;
	u8 month;
	u8 year;
	float Magnetic;//´ÅÆ«½Ç
	char Magnetic_dir;//E¶« W Î÷
	char mode;
}RMC_Data_t;//×î¾«¼òµÄPVTÊý¾Ý



typedef enum
{
	ANTENNA_OK =0,		//
	ANTENNA_OPEN,		//
	ANTENNA_CLOSE,
}ANTENNA_status_t;


typedef struct
{
	char Rxbuf[GPS_rx_len];
	char Txbuf[100];
	ANTENNA_status_t ANTENNA_status;    
	RMC_Data_t RMC_Data;
}GPS_object_t;

extern GPS_object_t GPS;

//extern GPS_Data_t gps_info_data;			//µ±Ç°¶¨Î»Êý¾Ý
//extern GPS_Data_t gps_info_data_old;	//Ö®Ç°¶¨Î»Êý¾Ý
/***********************************º¯ÊýÉùÃ÷************************************/
/**********************************************************
º¯ÊýÃû: GPS_data_get
Ãè  Êö: 2ms Ò»´Î
ÊäÈëÖµ: ÎÞ
Êä³öÖµ: ÎÞ
·µ»ØÖµ: ÎÞ
±¸  ×¢: ÎÞ
**********************************************************/
void GPS_data_get(void);
void GPS_RX(void);
#endif


