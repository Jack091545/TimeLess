#include "stc12c5a60s2.h"
#include "ds1302.h"
#include "Timer.h"
#include "data_store.h"

static unsigned char *time_data_buff;

static unsigned int nian,yue,ri,shi,fen,miao,zhou; /* 存储时钟显示值的全局变量 */

static unsigned char qian = 9,bai= 9,si = 9,ge=9;  /* 存储剩余天数显示值的全局变量 */

static const unsigned char  segout[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80}; /* 用于595对行的选择 */

static const unsigned char code number[10][8]={
{0x00,0x38,0x6C,0x6C,0x6C,0x6C,0x6C,0x38},/*"0",0*/

{0x00,0x18,0x1C,0x18,0x18,0x18,0x18,0x3C},/*"1",1*/

{0x00,0x38,0x6C,0x60,0x30,0x18,0x0C,0x7C},/*"2",2*/

{0x00,0x38,0x6C,0x60,0x38,0x60,0x6C,0x38},/*"3",3*/

{0x00,0x30,0x38,0x38,0x3C,0x3C,0x7C,0x30},/*"4",4*/

{0x00,0x7C,0x0C,0x0C,0x3C,0x60,0x60,0x3C},/*"5",5*/

{0x00,0x38,0x6C,0x0C,0x3C,0x6C,0x6C,0x38},/*"6",6*/

{0x00,0x7C,0x6C,0x60,0x30,0x18,0x18,0x18},/*"7",7*/

{0x00,0x38,0x6C,0x6C,0x38,0x6C,0x6C,0x38},/*"8",8*/

{0x00,0x38,0x6C,0x6C,0x78,0x60,0x6C,0x38}/*"9",9*/
};

static const unsigned char code lift_num[10][8]= {
{0x7C,0xFE,0x82,0xFE,0x7C,0x00,0x00,0x00},/*"0",0*/

{0x00,0x80,0xFE,0xFE,0x84,0x00,0x00,0x00},/*"1",1*/

{0x8C,0x9E,0xB2,0xE6,0xC4,0x00,0x00,0x00},/*"2",2*/

{0x6C,0xFE,0x92,0xD6,0x44,0x00,0x00,0x00},/*"3",3*/

{0x20,0xFE,0xFE,0x24,0x38,0x00,0x00,0x00},/*"4",4*/

{0x62,0xF2,0x92,0x9E,0x9E,0x00,0x00,0x00},/*"5",5*/

{0x64,0xF6,0x92,0xFE,0x7C,0x00,0x00,0x00},/*"6",6*/

{0x0E,0x1E,0xF2,0xE6,0x06,0x00,0x00,0x00},/*"7",7*/

{0x6C,0xFE,0x92,0xFE,0x6C,0x00,0x00,0x00},/*"8",8*/

{0x7C,0xFE,0x92,0xDE,0x4C,0x00,0x00,0x00}/*"9",9*/
};

static const unsigned char code lift_num_dot[10][8] = {
{0x6c,0x00,0x7C,0xFE,0x82,0xFE,0x7C,0x00},/*"0",0*/

{0x6c,0x00,0x00,0x80,0xFE,0xFE,0x84,0x00},/*"1",0*/

{0x6c,0x00,0x8C,0x9E,0xB2,0xE6,0xC4,0x00},/*"2",2*/

{0x6c,0x00,0x6C,0xFE,0x92,0xD6,0x44,0x00},/*"3",3*/

{0x6c,0x00,0x20,0xFE,0xFE,0x24,0x38,0x00},/*"4",4*/

{0x6c,0x00,0x62,0xF2,0x92,0x9E,0x9E,0x00},/*"5",5*/

{0x6c,0x00,0x64,0xF6,0x92,0xFE,0x7C,0x00},/*"6",6*/

{0x6c,0x00,0x0E,0x1E,0xF2,0xE6,0x06,0x00},/*"7",7*/

{0x6c,0x00,0x6C,0xFE,0x92,0xFE,0x6C,0x00},/*"8",8*/

{0x6c,0x00,0x7C,0xFE,0x92,0xDE,0x4C,0x00}/*"9",9*/
};

static const unsigned char code right_num_dot[10][8] = {
{0x00,0x7C,0xFE,0x82,0xFE,0x7C,0x00,0x6c},/*"0",0*/

{0x00,0x80,0xFE,0xFE,0x84,0x00,0x00,0x6c},/*"1",0*/

{0x00,0x8C,0x9E,0xB2,0xE6,0xC4,0x00,0x6c},/*"2",2*/

{0x00,0x6C,0xFE,0x92,0xD6,0x44,0x00,0x6c},/*"3",3*/

{0x00,0x20,0xFE,0xFE,0x24,0x38,0x00,0x6C},/*"4",4*/

{0x00,0x62,0xF2,0x92,0x9E,0x9E,0x00,0x6c},/*"5",5*/

{0x00,0x64,0xF6,0x92,0xFE,0x7C,0x00,0x6c},/*"6",6*/

{0x00,0x0E,0x1E,0xF2,0xE6,0x06,0x00,0x6c},/*"7",7*/

{0x00,0x6C,0xFE,0x92,0xFE,0x6C,0x00,0x6c},/*"8",8*/

{0x00,0x7C,0xFE,0x92,0xDE,0x4C,0x00,0x6c}/*"9",9*/

};

static const unsigned char code right_num[10][8] = {
{0x00,0x00,0x00,0x7C,0xFE,0x82,0xFE,0x7C},/*"0",0*/

{0x00,0x00,0x00,0x00,0x80,0xFE,0xFE,0x84},/*"1",1*/

{0x00,0x00,0x00,0x8C,0x9E,0xB2,0xE6,0xC4},/*"2",2*/

{0x00,0x00,0x00,0x6C,0xFE,0x92,0xD6,0x44},/*"3",3*/

{0x00,0x00,0x00,0x20,0xFE,0xFE,0x24,0x38},/*"4",4*/

{0x00,0x00,0x00,0x62,0xF2,0x92,0x9E,0x9E},/*"5",5*/

{0x00,0x00,0x00,0x64,0xF6,0x92,0xFE,0x7C},/*"6",6*/

{0x00,0x00,0x00,0x0E,0x1E,0xF2,0xE6,0x06},/*"7",7*/

{0x00,0x00,0x00,0x6C,0xFE,0x92,0xFE,0x6C},/*"8",8*/

{0x00,0x00,0x00,0x7C,0xFE,0x92,0xDE,0x4C}/*"9",9*/
};
/*------------------------------------------------
                硬件端口定义
------------------------------------------------*/

sbit LATCH = P2^6; 
sbit SRCLK= P2^5;
sbit SER  = P2^7;

//行信号控制
sbit LATCH_B = P2^3;
sbit SRCLK_B= P2^4;
sbit SER_B= P2^2;


/*------------------------------------------------
                发送字节程序
------------------------------------------------*/
static void SendByte(unsigned char dat)
{    
  unsigned char i; 
        
	for(i=0;i<8;i++)
	{
		SRCLK_B=0;
		SER_B=dat&0x80;
		dat<<=1;
		SRCLK_B=1;
	}
}
/*------------------------------------------------
                发送双字节序
     595级联，n个595，就需要发送n字节后锁存
------------------------------------------------*/
static void Send4Byte(unsigned char dat1,unsigned char dat2,unsigned char dat3,unsigned char dat4)
{    
   SendByte(dat1);
   SendByte(dat2);
   SendByte(dat3);
   SendByte(dat4);
   LATCH_B=0;
   LATCH_B=1;     
}

/*------------------------------------------------
	行信号
------------------------------------------------*/
static void SendSeg(unsigned char dat)
{    
unsigned char i; 
        
   for(i=0;i<8;i++)  //发送8行对应的数据
	{
		SRCLK=0;
		SER=dat&0x80;
		dat<<=1;
		SRCLK=1;
	}
	LATCH=0;    //锁存
	LATCH=1;
}

/*------------------------------------------------
	显示，从左到右
------------------------------------------------*/
static void Send_Data(unsigned char dat1[],unsigned char dat2[],unsigned char dat3[],unsigned char dat4[])
{
	unsigned char i = 0;
	for(i = 0;i < 8;i++)
	{
		SendSeg(~segout[i]); //0xfd
		Send4Byte(dat4[i],dat3[i],dat2[i],dat1[i]);
		Send4Byte(0x00,0x00,0x00,0x00);//delay(10); //防止重
	}
}

/*------------------------------------------------
	显示时钟函数
------------------------------------------------*/
static void Data_Deal(void)
{
	time_data_buff = Get_time_buf1();
	shi = time_data_buff[4];//时	
	fen = time_data_buff[5];//分
	miao = time_data_buff[6];//秒	
	nian = time_data_buff[1];//年									    
	yue = time_data_buff[2];//月
	ri = time_data_buff[3];//日	
	zhou = time_data_buff[7];//周
	Send_Data(&lift_num[shi/10][0],&lift_num_dot[shi%10][0],&right_num_dot[fen/10][0],&right_num[fen%10][0]);
}


/*------------------------------------------------
	显示剩余天数函数
------------------------------------------------*/
static void Data_Deal_Cry(void)
{
	unsigned int temp;
	static unsigned char flag = 0;
	if((shi == 23)&&(fen == 58))	 /* 如果时间到了23:58，那么就开始先读取剩余天数，然后减1，再写回去 */
	{
		if(flag == 0)
		{
			Set_Timeless(Get_Timeless() - 1);
			flag = 1;
		}

	}
	if((shi == 23)&&(fen == 59))	 /* 如果时间到了23:59，那么就把写时间的标志位给初始化 */
	{
		flag = 0;
	}
//	Set_Timeless(1988);
	temp = Get_Timeless() ;
	qian = (temp /1000)%10;
	bai = (temp /100)%10 ;
	si =  (temp /10)%10	;
	ge = temp %10	 ;
	Send_Data(&lift_num[qian][0],&lift_num[bai][0],&lift_num[si][0],&lift_num[ge][0]);
}


/*------------------------------------------------
	显示任务函数
------------------------------------------------*/
void Display_Task()
{
#ifdef __WEI__					   /* 如果定义了是魏远铭使用的版本 */
	if(Get_Display_Switch())
	{
		Data_Deal();
	}
	else
	{
		Data_Deal_Cry();
	}
#else
	Data_Deal();
#endif
	
}
