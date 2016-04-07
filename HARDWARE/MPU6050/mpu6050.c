#include "mpu6050.h"
#include  "sys.h"
#include "delay.h"

#define  uchar unsigned char 

#define  uint  unsigned int 

#define SMPLRT_DIV  0x19 //�����ǲ����ʪ�����ֵ��0x07(125Hz) 
#define CONFIG   0x1A //��ͨ�˲�Ƶ�ʪ�����ֵ��0x06(5Hz) 
#define GYRO_CONFIG  0x1B //�������Լ켰������Χ������ֵ��0x18(���Լ쪢2000deg/s) 
#define ACCEL_CONFIG 0x1C //���ټ��Լ졢������Χ����ͨ�˲�Ƶ�ʪ�����ֵ��0x01(���Լ쪢2G��5Hz) 
#define ACCEL_XOUT_H 0x3B 
#define ACCEL_XOUT_L 0x3C 
#define ACCEL_YOUT_H 0x3D 
#define ACCEL_YOUT_L 0x3E 
#define ACCEL_ZOUT_H 0x3F 
#define ACCEL_ZOUT_L 0x40 
#define TEMP_OUT_H  0x41 
#define TEMP_OUT_L  0x42 
#define GYRO_XOUT_H  0x43 
#define GYRO_XOUT_L 0x44  
#define GYRO_YOUT_H  0x45 
#define GYRO_YOUT_L  0x46 
#define GYRO_ZOUT_H  0x47 
#define GYRO_ZOUT_L  0x48 
#define PWR_MGMT_1  0x6B //��Դ����������ֵ��0x00(��������) 
#define WHO_AM_I   0x75 //IIC��ַ�Ĵ���(Ĭ����ֵ0x68��ֻ��) 
#define SlaveAddress 0xD0 //IICд��ʱ�ĵ�ַ�ֽ����ݪ�+1Ϊ��ȡ 


//IO��������
//PB.7 -> SDA
#define MPU_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;} 
#define MPU_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}

//IO��������	 
#define MPU_IIC_SCL    PBout(6) 		//SCL
#define MPU_IIC_SDA    PBout(7) 		//SDA	 
#define MPU_READ_SDA   PBin(7) 		//����SDA 

	
#define SCL PBout(6) 		//SCL
#define SDA PBout(7) 		//SDA


int Accel_x,Accel_y,Accel_z ;
float Gyro_y;

float x_zero=0.0,y_zero=0.0,z_zero=0.0;

int Angle_Speed;

short Left_DeadMotor = 36,Right_DeadMotor = 36;

uchar value;

int buchang=0,buchang_L = 0;

float Lx,Rx;

//******����������************
float Q_angle=0.001, Q_gyro=0.003, R_angle=0.5, dt=0.005;//ע�⣺dt��ȡֵΪkalman�˲�������ʱ��;
float PP[2][2] = {{1.0,0.0},{0.0,1.0}};
float Pdot[4]={0,0,0,0};
const float C_0 = 1;
float Q_bias = 0, Angle_err = 0, PCt_0, PCt_1, E, K_0, K_1, t_0, t_1;
char cy; 


//������
void delayM(unsigned int t) 
{       
   delay_us(2);
}          

//��ʼ��ģ��IIC�˿�����
void MPU_IIC_Init(void)
{					     
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//��ʹ������IO PORTBʱ�� 
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;// �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);				//�����趨������ʼ��GPIO 

	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);//PB6,PB7 �����	
}


void sda_high()    //SDA�ߵ�ƽ�Ĳ��� 

{ 
    MPU_SDA_OUT();

	MPU_IIC_SDA=1; 

    _asm(nop);                     

    _asm(nop); 

    _asm(nop);                    

    _asm(nop); 

} 

//************************************** 

//SDA�͵�ƽ�Ĳ��� 

//************************************** 

void sda_low()   //SDA�͵�ƽ�Ĳ��� 

{ 

    MPU_SDA_OUT();

    MPU_IIC_SDA=0; 

    _asm(nop);                     

    _asm(nop); 

    _asm(nop);                    

    _asm(nop);   
                        
} 

//************************************** 

//SCL�ߵ�ƽ���� 

//************************************** 

void scl_high()  //SCL�ߵ�ƽ���� 

{ 
    MPU_SDA_OUT();

    MPU_IIC_SCL=1; 

    _asm(nop);                     

    _asm(nop); 

    _asm(nop);                    

    _asm(nop); 

} 

//************************************** 

//SCL�͵�ƽ�Ĳ��� 

//************************************** 

void scl_low() //SCL�͵�ƽ�Ĳ��� 

{ 
    MPU_SDA_OUT();

    MPU_IIC_SCL=0; 

    _asm(nop);                     

    _asm(nop); 

    _asm(nop);                    

    _asm(nop); 
} 

//************************************** 

//I2C��ʼ�ź� 

//************************************** 

void I2C_Start() 

{ 

    sda_high();                    //���������� 

    scl_high();                    //����ʱ���� 

    delayM(1);                 //��ʱ 

    sda_low();                    //�����½��� 

    delayM(1);                 //��ʱ 

    scl_low();                    //����ʱ���� 

} 

//************************************** 

//I2Cֹͣ�ź� 

//************************************** 

void I2C_Stop() 
{ 

    sda_low();                    //���������� 

    scl_high();                    //����ʱ���� 

    delayM(1);                 //��ʱ 

    sda_high();                    //���������� 

    delayM(1);                //��ʱ 

} 

//************************************** 

//I2C����Ӧ���ź� 

//��ڲ���:ack (0:ACK 1:NAK) 

//************************************** 

void I2C_SendACK(char ack) 

{    

    if(ack)   sda_high(); 

    else    

     sda_low(); //дӦ���ź� 

     scl_high();                    //����ʱ���� 

     delayM(1);                   //��ʱ 

     scl_low();                    //����ʱ���� 

     delayM(1);                   //��ʱ 

} 

//************************************** 

//I2C����Ӧ���ź� 

//************************************** 

void I2C_RecvACK() 

{    

	MPU_SDA_IN();
    scl_high();                    //����ʱ���� 

    MPU_SDA_IN();

    delayM(1);                //��ʱ 

    if(MPU_IIC_SDA==1) 

    { 
        cy=1; 
    } 

    else  

    { 
        cy=0; 
    } 

//    cy = SDA;                   //��Ӧ���ź� 

    scl_low();                    //����ʱ���� 

    delayM(1);                //��ʱ 

    MPU_SDA_OUT();  
    
 //   return cy; 

} 

//************************************** 

//��I2C���߷���һ���ֽ����� 

//************************************** 

void I2C_SendByte(uchar dat) 

{ 

    uchar i; 

    for (i=0; i<8; i++)         //8λ������ 

    { 

        if((dat<<i)&0x80) 

        { 

            sda_high(); 

        } 

        else  

        { 

            sda_low(); 

        } 

       // SDA = cy;               //�����ݿ� 

        scl_high();                //����ʱ���� 

        delayM(1);              //��ʱ 

        scl_low();                //����ʱ���� 

        delayM(1);              //��ʱ 

    } 

    I2C_RecvACK(); 

} 

//************************************** 

//��I2C���߽���һ���ֽ����� 

//************************************** 

uchar I2C_RecvByte() 

{ 

    uchar i; 

    uchar dat = 0,cy; 

    sda_high();                    //ʹ���ڲ�����,׼����ȡ����, 

	MPU_SDA_IN();
	
    for (i=0; i<8; i++)         //8λ������ 

    { 

        dat <<= 1; 

        scl_high();                //����ʱ���� 

        delayM(1);             //��ʱ 

        if(MPU_READ_SDA==1) 

        { 

            cy=1;         
        } 

        else  

        { 

            cy=0; 

        } 

        dat |= cy;             //������              

        scl_low();                //����ʱ���� 
                       
        delayM(1);             //��ʱ 

    } 

	MPU_SDA_OUT();
    return dat; 

} 

//************************************** 

//��I2C�豸д��һ���ֽ����� 

//************************************** 

void Single_WriteI2C(uchar REG_Address,uchar REG_data) 

{ 

    I2C_Start();                  //��ʼ�ź� 

    I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź� 

    I2C_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ�� 

    I2C_SendByte(REG_data);       //�ڲ��Ĵ������ݪ� 

    I2C_Stop();                   //����ֹͣ�ź� 

} 

//************************************** 

//��I2C�豸��ȡһ���ֽ����� 

//************************************** 

uchar Single_ReadI2C(uchar REG_Address) 

{ 

   uchar REG_data; 

   I2C_Start();                   //��ʼ�ź� 

   I2C_SendByte(SlaveAddress);    //�����豸��ַ+д�ź� 

   I2C_SendByte(REG_Address);     //���ʹ洢��Ԫ��ַ����0��ʼ  

   I2C_Start();                   //��ʼ�ź� 

   I2C_SendByte(SlaveAddress+1);  //�����豸��ַ+���ź� 

   REG_data=I2C_RecvByte();       //�����Ĵ������� 

   I2C_SendACK(1);                //����Ӧ���ź� 

   I2C_Stop();                    //ֹͣ�ź� 

   return REG_data; 

} 

//************************************** 

//��ʼ��MPU6050 

//************************************** 

void InitMPU6050() 

{ 
   Single_WriteI2C(PWR_MGMT_1, 0x00); //�������״̬ 

   Single_WriteI2C(SMPLRT_DIV, 0x07); 

   Single_WriteI2C(CONFIG, 0x06); 

   Single_WriteI2C(GYRO_CONFIG, 0x18); 

   Single_WriteI2C(ACCEL_CONFIG, 0x01); 

} 

//************************************** 

//�ϳ����� 

//************************************** 

short GetData(uchar REG_Address) 

{ 

   uchar H,L; 

   H=Single_ReadI2C(REG_Address); 

   L=Single_ReadI2C(REG_Address+1); 

   return (H<<8)+L;   //�ϳ����� 

} 


float MPU6050_Get_Angle(float x,float z) //,float y,
{ 

    float temp , res; 
 
    temp=x/sqrt(/*y*y+*/z*z); 

    res=atan(temp); 

    return res*180.0/3.1415926;//�ѻ���ת���ɽǶ� 
/**************
    ����ȻZ��ĽǶ�
        temp=sqrt((x*x+y*y))/z; 

        res=atan(temp); 
        
    ����ȻY��ĽǶ� 
        temp=y/sqrt((x*x+z*z)); 

        res=atan(temp); 
***************/
} 


                                  
void Get_Gyro_Zero(void) 
{
    int i;
    gyro_zero=0;
    for(i=0;i<1000;i++) 
    {
        gyro_zero += (unsigned int)GetData(GYRO_YOUT_H); //Y����ٶ�
    }  
    gyro_zero = (gyro_zero/1000); 
}

void Get_Gravity_Zero(void) 
{
    int i;
    for(i=0;i<1000;i++) 
    {
        x_zero +=GetData(ACCEL_XOUT_H)*0.001; //X�����ֵ����ת��Ϊ�Ƕ� 

        y_zero +=GetData(ACCEL_YOUT_H)*0.001; 

        z_zero +=GetData(ACCEL_ZOUT_H)*0.001; 
    }
}

// �������˲�
//*********************************************************
//Kalman�˲���20MHz�Ĵ���ʱ��Լ0.77ms��
void Kalman_Filter(float Accel,float Gyro)		
{
	Angle+=(Gyro - Q_bias) * dt; //�������
  Pdot[0]=Q_angle - PP[0][1] - PP[1][0]; // Pk-����������Э�����΢��
	Pdot[1]=- PP[1][1];
	Pdot[2]=- PP[1][1];
	Pdot[3]=Q_gyro;
	PP[0][0] += Pdot[0] * dt;   // Pk-����������Э����΢�ֵĻ���
	PP[0][1] += Pdot[1] * dt;   // =����������Э����
	PP[1][0] += Pdot[2] * dt;
	PP[1][1] += Pdot[3] * dt;
	Angle_err = Accel - Angle;	//zk-�������
	PCt_0 = C_0 * PP[0][0];
	PCt_1 = C_0 * PP[1][0];
	E = R_angle + C_0 * PCt_0;
	K_0 = PCt_0 / E;
	K_1 = PCt_1 / E;	
	t_0 = PCt_0;
	t_1 = C_0 * PP[0][1];
	PP[0][0] -= K_0 * t_0;		 //����������Э����
	PP[0][1] -= K_0 * t_1;
	PP[1][0] -= K_1 * t_0;
	PP[1][1] -= K_1 * t_1;		
	Angle	+= K_0 * Angle_err;	 //�������
	Q_bias	+= K_1 * Angle_err;	 //�������
//Gyro_y   = Gyro - Q_bias;	 //���ֵ(�������)��΢��=���ٶ�
//Angle_gy = Angle_gy + Gyro_y*0.005 ;  //���ٶ�ֵ���ּ���Ƕ�
}

// ��Ǽ��㣨�������ںϣ�
void Angle_Calcu(void)	 
{
	//------���ٶ�--------------------------
	//��ΧΪ2gʱ�������ϵ��16384 LSB/g
	//�ǶȽ�Сʱ��x=sinx�õ��Ƕȣ����ȣ�, deg = rad*180/3.14
	//��Ϊx>=sinx,�ʳ���1.3�ʵ��Ŵ�
	
	  Accel_x = (GetData(ACCEL_XOUT_H)-Gravity_X_Zero); //X����ٶȪ���ת��Ϊ�Ƕ� 

    Accel_y = (GetData(ACCEL_YOUT_H)-Gravity_Y_Zero); //��ȥ��� 

    Accel_z = (GetData(ACCEL_ZOUT_H)-Gravity_Z_Zero);  
    
    Gyro_y=-((GetData(0x45)-gyro_zero))/16.4; //������Y����ٶ� ,��ȥ���Ư�� 

    Angle_ax=MPU6050_Get_Angle( Accel_x,Accel_z );//Accel_y,//�ɼ��ٶȼ�����Ƕ�
    
  //Get_MMA7361();
	
//	Accel_x  = GetData(ACCEL_XOUT_H);	  //��ȡX����ٶ�
//	Angle_ax = (Accel_x - 1100) /16384;   //ȥ�����ƫ��,����õ��Ƕȣ����ȣ�
//	Angle_ax = Angle_ax*1.2*180/3.14;     //����ת��Ϊ��,
    //-------���ٶ�-------------------------
	//��ΧΪ2000deg/sʱ�������ϵ��16.4 LSB/(deg/s)
 
//	 Angle_gy = Angle_gy + Gyro_y*0.01;  //���ٶȻ��ֵõ���б�Ƕ�.		
	//-------�������˲��ں�-----------------------
	
//	Kalman_Filter(Angle_ax,Gyro_y);       //�������˲��������
	
	/*//-------�����˲�-----------------------
	//����ԭ����ȡ��ǰ��Ǻͼ��ٶȻ����ǲ�ֵ���зŴ�Ȼ����
    //�����ǽ��ٶȵ��Ӻ��ٻ��֣��Ӷ�ʹ��������Ϊ���ٶȻ�õĽǶ�
	//0.5Ϊ�Ŵ������ɵ��ڲ����ȣ�0.01Ϊϵͳ����10ms			
	Angle = Angle + (((Angle_ax-Angle)*0.5 + Gyro_y)*0.01);*/															  
} 
void Show_Angle(void) 
{
   
    /*
         OutData[0] =  (short)Angle_ax*100;  //���ٶȼ��������ýǶ�
         OutData[1] =  (short)Gyro_y*100;    //���ٶ���ֵ
         OutData[2] =  (short)Angle*100;     //�������˲�����Ƕ�
    */
     /*
         ���ٶȻ������ýǶȣ���������֤����������     
         OutData[3] = (short) Angle_gy*100; 
         trans((int)Angle_gy,a) ;
         LCD_P8x16Str(5,6,a);
     */
         trans((int)Angle_ax,x);
         trans((int)Angle,y);
         trans((int)Gyro_y,z) ;
                           
         LCD_P8x16Str(5,0,x); //LED��ʾ
         LCD_P8x16Str(45,0,y);
       //  LCD_P8x16Str(5,2,z);

   //      OutPut_Data(); //�����������
}



/*
����������

void Get_Gravity(void)
{
    
   
   
    x1=GetData(ACCEL_XOUT_H); X�����ֵ����ת��Ϊ�Ƕ� 

    y1=GetData(ACCEL_YOUT_H);  

    z1=GetData(ACCEL_ZOUT_H);  

    Accel_x=MPU6050_Get_Angle( x1, y1, z1,0);

    Accel_y=MPU6050_Get_Angle( x1, y1, z1,1);

    Accel_z=MPU6050_Get_Angle( x1, y1, z1,2);
    
    trans(Accel_x,x) ;
    trans(Accel_y,y) ;
    trans(Accel_z,z) ;
    
     LCD_P8x16Str(5,0,x);
     LCD_P8x16Str(5,2,y);
     LCD_P8x16Str(5,4,z);
}



void Get_Gyro(void) 
{
   
    
    Gyro_x=GetData(GYRO_XOUT_H); ��ʾX����ٶ� 
       
    Gyro_y=GetData(GYRO_YOUT_H); ��ʾY����ٶ� 

    Gyro_z=GetData(GYRO_ZOUT_H); ��ʾZ����ٶ� 
}

*/
