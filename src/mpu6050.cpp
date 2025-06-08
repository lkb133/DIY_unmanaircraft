#include "mpu6050.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/**********************************************
函数名称：MPU_Init
函数功能：初始化MPU6050
函数参数：无
函数返回值：0,初始化成功  其他,初始化失败
**********************************************/
u8 MPU_Init(void)
{
  u8 res;
  i2c_master_init();                       // 初始化IIC总线
  MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); // 复位MPU6050
  vTaskDelay(pdMS_TO_TICKS(100));

  MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); // 唤醒MPU6050
  MPU_Set_Gyro_Fsr(3);                     // 陀螺仪传感器,±2000dps
  MPU_Set_Accel_Fsr(0);                    // 加速度传感器,±2g
  MPU_Set_Rate(50);                        // 设置采样率50Hz
  // MPU_Write_Byte(MPU_INT_EN_REG, 0X00);    // 关闭所有中断
  MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C主模式关闭
  MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);   // 关闭FIFO
  MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT引脚低电平有效

  res = MPU_Read_Byte(MPU_DEVICE_ID_REG);
  if (res == MPU_ADDR) // 器件ID正确,即res = MPU_ADDR = 0x68
  {
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); // 设置CLKSEL,PLL X轴为参考
    MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); // 加速度与陀螺仪都工作
    MPU_Set_Rate(50);                        // 设置采样率为50Hz
  }
  else
    return 1; // 地址设置错误,返回1
  return 0;   // 地址设置正确,返回0
}

/**********************************************
函数名称：MPU_Set_Gyro_Fsr
函数功能：设置MPU6050陀螺仪传感器满量程范围
函数参数：fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
函数返回值：0,设置成功  其他,设置失败
**********************************************/
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
  return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3); // 设置陀螺仪满量程范围
}

/**********************************************
函数名称：MPU_Set_Accel_Fsr
函数功能：设置MPU6050加速度传感器满量程范围
函数参数：fsr:0,±2g;1,±4g;2,±8g;3,±16g
函数返回值：0,设置成功  其他,设置失败
**********************************************/
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
  return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3); // 设置加速度传感器满量程范围
}

/**********************************************
函数名称：MPU_Set_LPF
函数功能：设置MPU6050的数字低通滤波器
函数参数：lpf:数字低通滤波频率(Hz)
函数返回值：0,设置成功  其他,设置失败
**********************************************/
u8 MPU_Set_LPF(u16 lpf)
{
  u8 data = 0;

  if (lpf >= 188)
    data = 1;
  else if (lpf >= 98)
    data = 2;
  else if (lpf >= 42)
    data = 3;
  else if (lpf >= 20)
    data = 4;
  else if (lpf >= 10)
    data = 5;
  else
    data = 6;
  return MPU_Write_Byte(MPU_CFG_REG, data); // 设置数字低通滤波器
}

/**********************************************
函数名称：MPU_Set_Rate
函数功能：设置MPU6050的采样率(假定Fs=1KHz)
函数参数：rate:4~1000(Hz)  初始化中rate取50
函数返回值：0,设置成功  其他,设置失败
**********************************************/
u8 MPU_Set_Rate(u16 rate)
{
  u8 data;
  if (rate > 1000)
    rate = 1000;
  if (rate < 4)
    rate = 4;
  data = 1000 / rate - 1;
  data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data); // 设置数字低通滤波器
  return MPU_Set_LPF(rate / 2);                     // 自动设置LPF为采样率的一半
}

/**********************************************
函数名称：MPU_Get_Temperature
函数功能：得到温度传感器值
函数参数：无
函数返回值：温度值(扩大了100倍)
**********************************************/
short MPU_Get_Temperature(void)
{
  u8 buf[2];
  short raw;
  float temp;

  MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
  raw = ((u16)buf[0] << 8) | buf[1];
  temp = 36.53 + ((double)raw) / 340;
  return temp * 100;
}

/**********************************************
函数名称：MPU_Get_Gyroscope
函数功能：得到陀螺仪值(原始值)
函数参数：gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
函数返回值：0,读取成功  其他,读取失败
**********************************************/
u8 MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
  u8 buf[6], res;

  res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
  if (res == 0)
  {
    *gx = ((u16)buf[0] << 8) | buf[1];
    *gy = ((u16)buf[2] << 8) | buf[3];
    *gz = ((u16)buf[4] << 8) | buf[5];
  }
  return res;
}

/**********************************************
函数名称：MPU_Get_Accelerometer
函数功能：得到加速度值(原始值)
函数参数：ax,ay,az:加速度传感器x,y,z轴的原始读数(带符号)
函数返回值：0,读取成功  其他,读取失败
**********************************************/
u8 MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
  u8 buf[6], res;
  res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
  if (res == 0)
  {
    *ax = ((u16)buf[0] << 8) | buf[1];
    *ay = ((u16)buf[2] << 8) | buf[3];
    *az = ((u16)buf[4] << 8) | buf[5];
  }
  return res;
}

/**********************************************
函数名称：MPU_Write_Len
函数功能：IIC连续写(写器件地址、寄存器地址、数据)
函数参数：addr:器件地址      reg:寄存器地址
        len:写入数据的长度  buf:数据区
函数返回值：0,写入成功  其他,写入失败
**********************************************/
u8 MPU_Write_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
  u8 i;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  esp_err_t ack = i2c_master_write_byte(cmd, (addr << 1) | 0, 1); // 发送器件地址+写命令(0为写,1为读)
  if (ack == ESP_ERR_NO_MEM)                                      // 等待应答
  {
    i2c_master_stop(cmd);
    return 1;
  }

  i2c_master_write_byte(cmd, reg, 1); // 写寄存器地址
  for (i = 0; i < len; i++)
  {
    esp_err_t ack1 = i2c_master_write_byte(cmd, buf[i], 1); // 发送数据
    if (ack1 == ESP_ERR_NO_MEM)                             // 等待应答
    {
      i2c_master_stop(cmd);
      return 1;
    }
  }
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(MPU_I2C_POER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return 0;
}

/**********************************************
函数名称：MPU_Read_Len
函数功能：IIC连续读(写入器件地址后,读寄存器地址、数据)
函数参数：addr:器件地址        reg:要读的寄存器地址
len:要读取的数据长度  buf:读取到的数据存储区
函数返回值：0,读取成功  其他,读取失败
**********************************************/
u8 MPU_Read_Len(u8 addr, u8 reg, u8 len, u8 *buf)
{
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  esp_err_t ack = i2c_master_write_byte(cmd, (addr << 1) | 0, 1); // 发送器件地址+写命令
  if (ack == ESP_ERR_NO_MEM)                                      // 等待应答
  {
    i2c_master_stop(cmd);
    return 1;
  }
  i2c_master_write_byte(cmd, reg, 1); // 写寄存器地址

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (addr << 1) | 1, 1); // 发送器件地址+读命令
  while (len)
  {
    if (len == 1)
      i2c_master_read_byte(cmd, buf, I2C_MASTER_NACK); // 读数据,发送nACK
    else
      i2c_master_read_byte(cmd, buf, I2C_MASTER_ACK); // 读数据,发送ACK
    len--;
    buf++;
  }
  i2c_master_stop(cmd); // 产生一个停止条件
  i2c_master_cmd_begin(MPU_I2C_POER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return 0;
}

/**********************************************
函数名称：MPU_Write_Byte
函数功能：IIC写一个字节
函数参数：data:写入的数据    reg:要写的寄存器地址
函数返回值：0,写入成功  其他,写入失败
**********************************************/
u8 MPU_Write_Byte(u8 reg, u8 data)
{
  // 开启i2c总线
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  esp_err_t ack = i2c_master_write_byte(cmd, (MPU_ADDR << 1) | 0, true); // 发送器件地址+写命令
  if (ack == ESP_ERR_NO_MEM)                                             // 等待应答
  {
    i2c_master_stop(cmd);
    return 1;
  }
  i2c_master_write_byte(cmd, reg, 1);                   // 写寄存器地址
  esp_err_t ack1 = i2c_master_write_byte(cmd, data, 1); // 发送数据
  if (ack1 == ESP_ERR_NO_MEM)                           // 等待ACK
  {
    i2c_master_stop(cmd);
    return 1;
  }
  i2c_master_stop(cmd);
  i2c_master_cmd_begin(MPU_I2C_POER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  return 0;
}

/**********************************************
函数名称：MPU_Read_Byte
函数功能：IIC读一个字节
函数参数：reg:要读的寄存器地址
函数返回值：res:读取到的数据
**********************************************/
u8 MPU_Read_Byte(u8 reg)
{
  u8 res;
  // 开启i2c总线
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (MPU_ADDR << 1) | 0, 1); // 发送器件地址+写命令
  i2c_master_write_byte(cmd, reg, 1);                 // 写寄存器地址

  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (MPU_ADDR << 1) | 1, 1); // 发送器件地址+读命令

  i2c_master_read_byte(cmd, &res, I2C_MASTER_NACK);

  i2c_master_stop(cmd);                                             // 产生一个停止条件
  i2c_master_cmd_begin(MPU_I2C_POER_NUM, cmd, pdMS_TO_TICKS(1000)); // 执行 I2C 命令
  i2c_cmd_link_delete(cmd);
  return res;
}

// i2c驱动安装
esp_err_t i2c_master_init(void)
{
  int i2c_master_port = MPU_I2C_POER_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = MPU_I2C_SDA;
  conf.scl_io_num = MPU_I2C_SCL;
  conf.sda_pullup_en = GPIO_PULLDOWN_ENABLE;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = MPU_I2C_FREQ;

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}