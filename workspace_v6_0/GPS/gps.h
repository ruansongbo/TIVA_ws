/*
 * gps.h
 *
 *  Created on: 2015��6��7��
 *      Author: ideapad
 */

#ifndef GPS_H_
#define GPS_H_
#include <stdint.h>
#include <stdbool.h>

//GPS NMEA-0183Э����Ҫ�����ṹ�嶨��
//������Ϣ
typedef struct
{
    uint8_t num;     //���Ǳ��
    uint8_t eledeg;  //��������
    uint16_t azideg; //���Ƿ�λ��
    uint8_t sn;      //�����
}nmea_slmsg;
//UTCʱ����Ϣ
typedef struct
{
    uint16_t year;   //���
    uint8_t month;   //�·�
    uint8_t date;    //����
    uint8_t hour;    //Сʱ
    uint8_t min;     //����
    uint8_t sec;     //����
}nmea_utc_time;
//NMEA 0183 Э����������ݴ�Žṹ��
typedef struct
{
    uint8_t svnum;                   //�ɼ�������
    nmea_slmsg slmsg[12];       //���12������
    nmea_utc_time utc;          //UTCʱ��
    uint32_t latitude;               //γ�� ������100000��,ʵ��Ҫ����100000
    uint8_t nshemi;                  //��γ/��γ,N:��γ;S:��γ
    uint32_t longitude;              //���� ������100000��,ʵ��Ҫ����100000
    uint8_t ewhemi;                  //����/����,E:����;W:����
    uint8_t gpssta;                  //GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.
    uint8_t posslnum;                //���ڶ�λ��������,0~12.
    uint8_t possl[12];               //���ڶ�λ�����Ǳ��
    uint8_t fixmode;                 //��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
    uint16_t pdop;                   //λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
    uint16_t hdop;                   //ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
    uint16_t vdop;                   //��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0

    int altitude;               //���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m
    uint16_t speed;                  //��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ
}nmea_msg;

int NMEA_Str2num(uint8_t *buf,uint8_t*dx);
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,uint8_t *buf);

#endif /* GPS_H_ */