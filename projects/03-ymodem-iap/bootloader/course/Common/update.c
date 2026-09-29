#include "update.h"
#include <stdio.h>
#include <string.h>
#include "systick.h"
#include "USART.h"

#include "flash_drv.h"

#define IS_AF(c)  					((c >= 'A') && (c <= 'F'))
#define IS_af(c)  					((c >= 'a') && (c <= 'f'))
#define IS_09(c)  					((c >= '0') && (c <= '9'))
#define ISVALIDHEX(c)  				(IS_AF(c) || IS_af(c) || IS_09(c))
#define ISVALIDDEC(c)  				IS_09(c)
#define CONVERTDEC(c)  				(c - '0')

#define CONVERTHEX_alpha(c)  (IS_AF(c) ? (c - 'A'+10) : (c - 'a'+10))
#define CONVERTHEX(c)   (IS_09(c) ? (c - '0') : CONVERTHEX_alpha(c))
/**
  * @brief  Convert a string to an integer
  * @param  inputstr: The string to be converted
  * @param  intnum: The intger value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
  {
    if (inputstr[2] == '\0')
    {
      return 0;
    }
    for (i = 2; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1; */
        res = 1;
        break;
      }
      if (ISVALIDHEX(inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* over 8 digit hex --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }
  else /* max 10-digit decimal input */
  {
    for (i = 0; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1 */
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
      {
        val = val << 10;
        *intnum = val;
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
      {
        val = val << 20;
        *intnum = val;
        res = 1;
        break;
      }
      else if (ISVALIDDEC(inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 10 digit decimal --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }

  return res;
}
/******************************************************************************
 * Name:    CRC-16/XMODEM       x16+x12+x5+1
 * Poly:    0x1021
 * Init:    0x0000
 * Refin:   False
 * Refout:  False
 * Xorout:  0x0000
 * Alias:   CRC-16/ZMODEM,CRC-16/ACORN
 *****************************************************************************/
uint16_t Crc16Ymodem(uint8_t *data, uint16_t length)
{
  uint8_t i;
  uint16_t crc = 0;            // Initial value
  while (length--)
  {
    crc ^= (uint16_t)(*data++) << 8;
    for (i = 0; i < 8; ++i)
    {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc <<= 1;
    }
  }
  return crc;
}

// ====================================================================

// 实现真正的数据接收，烧录过程（阻塞实现）
#define YMODEM_PACKET_LENGTH        1024
static uint8_t g_packetBuffer[YMODEM_PACKET_LENGTH];

#define FILE_NAME_LENGTH           	256
#define FILE_SIZE_LENGTH           	16

#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define SOH                     (0x01)  //128字节数据包开始
#define STX                     (0x02)  //1024字节的数据包开始
#define EOT                     (0x04)  //结束传输
#define ACK                     (0x06)  //回应
#define NAK                     (0x15)  //空回应
#define CA                      (0x18)  //这两个相继中止转移
#define CREQ                    (0x43)  //'C' == 0x43, 请求数据

#define ABORT1                  (0x41)  //'A' == 0x41, 用户终止 
#define ABORT2                  (0x61)  //'a' == 0x61, 用户终止

#define NAK_TIMEOUT							(0x100000)
#define MAX_ERRORS              (65565)

// 文件名
char g_imageName[FILE_NAME_LENGTH];


/**********************************************************
 * @brief 读取文件信息
 * @param packetData IN 要解析的数据
 * @param size OUT 文件大小
 * @return 
 **********************************************************/
void getFileInfo(uint8_t* packetData, int32_t* size){
		int32_t i;
		uint8_t fileSize[FILE_SIZE_LENGTH];
	
		uint8_t *filePtr;
		/* 跳过3个字节，取出文件名，直到遇到0x00(\0), Filename packet has valid data */
		for (i = 0, filePtr = packetData + PACKET_HEADER; (*filePtr != '\0') && (i < FILE_NAME_LENGTH);)
		{
			g_imageName[i++] = *filePtr++;
		}
		g_imageName[i++] = '\0';

		/* 取出数据总长度，直到遇到空字符 */
		for (i = 0, filePtr ++; (*filePtr != ' ') && (i < FILE_SIZE_LENGTH);)
		{
			fileSize[i++] = *filePtr++;
		}
		fileSize[i++] = '\0';
		// 将数字字符串转成int32_t类型数字
		Str2Int(fileSize, size);
}

static char str_buf[100] = {0};

static int32_t ReceivePacket(uint8_t* data, int32_t *length, uint32_t timeout)
{
	uint16_t packetSize = 0, i;
	*length = 0; // 一定要初始化数据长度为0
	
	// 获取消息头
	uint8_t c; 
	if(ReceiveByteTimeout(&c, timeout) != 0){
		return -10;
	}
	
//	sprintf(str_buf, "<0x%02X> ", c); 这里打日志会影响数据接收
//	USART1_send_string(str_buf);
	
	// 解析消息头
	switch(c){
		case SOH: // 0x01
			packetSize = PACKET_SIZE;
			break;
		case STX:	// 0x02
			packetSize = PACKET_1K_SIZE;
			break;
		case EOT:	// 0x04 结束
			return 0;
		case CA: // 发送者结束发送 Ctrl + C
			if(ReceiveByteTimeout(&c, timeout) == 0 && (c == CA)){
				*length = -1;
				return 0;
			}else {
				return -11;
			}
		
		case ABORT1:	// 发送端主动停止传送 A
		case ABORT2:  // 发送端主动停止传送 a
			return 1;
		default:
			return -1;	// 接收包头错误
	}
	*data = c;
	// 循环接收N个数据
	for(i = 1; i < (packetSize + PACKET_OVERHEAD); i++){ // 128 + 5 == 133
		// 将数据接收到data索引为i的位置 &data[i]
		if(ReceiveByteTimeout(data + i, timeout) != 0){
			USART1_send_string("timeout");
			return -2;	// 接收数据超时
		}
	}
	sprintf(str_buf, "head: %02X %02X %02X %02X crc: %02X %02X\n", 
			data[0], data[1], data[2], data[3], data[packetSize + PACKET_OVERHEAD - 2], data[packetSize + PACKET_OVERHEAD - 1]);
	USART1_send_string(str_buf);
	
	// 校验序列号和反序列号
	if((data[PACKET_SEQNO_INDEX] | data[PACKET_SEQNO_COMP_INDEX]) != 0xFF){
		return -3;
	}
	
	// 校验CRC16: 实际值和期望值一致
	uint16_t actual_crc = Crc16Ymodem(&data[3], packetSize);
	uint16_t except_crc = (data[packetSize + PACKET_OVERHEAD - 2] << 8) | data[packetSize + PACKET_OVERHEAD - 1];
	if(actual_crc != except_crc){
		return -4;
	}
	
	*length = packetSize;
	return 0;
}

#define SendByte(dat)				USART0_send_byte(dat);

/**********************************************************
 * @brief 通过Ymodem协议接收文件，并写入到Flash的App区域
 * @param buf 数据缓存数组
 * @return 接收到的固件大小（单位：字节）
 **********************************************************/
int32_t YmodemReceive(uint8_t *buf){
	// packetData用于接收每一包数据
	uint8_t packetData[PACKET_1K_SIZE + PACKET_OVERHEAD], *bufPtr;
	// 数据包字节数，数据包索引，固件总字节个数
	int32_t packetLength, packetsReceived = 0, size = 0;
	// 会话开始标记，会话结束标记，文件结束标记，错误次数
	int32_t sessionBegin = 0, sessionDone = 0,fileDone = 0, errors = 0;
	
	// 目标位置
	uint32_t flashDestination = APP_ADDR_IN_FLASH;
	
	// 大循环：接收一个会话session的多个文件
	while(1){
		
		flashDestination = APP_ADDR_IN_FLASH; // 可选，重置目标Flash地址
		// 小循环：接收一个文件File的多个数据包
		// 阻塞式接收多个数据包
		for(packetsReceived = 0, fileDone = 0, bufPtr = buf;;){
			
			int rst = ReceivePacket(packetData, &packetLength, NAK_TIMEOUT);
			sprintf(str_buf, "rst: %d len: %d\n", rst, packetLength);
			USART1_send_string(str_buf);
			
	//		sprintf(str_buf, "errors: %d  rst: %d\n", errors, rst);
	//    USART1_send_string(str_buf);
			
			if(rst == 1){
				// 发送端主动中止发送, 确认结束
				SendByte(CA);
				SendByte(CA);
				return -3;
			}

			// 每收到一个数据包，解析，写入，响应ACK
			if(rst != 0){
				if(sessionBegin > 0){ // 已经开始传输了
					errors++;
				}
				
				if(errors > MAX_ERRORS){	// 错误次数过多
					// 发送两个取消指令
					SendByte(CA);
					SendByte(CA);
					return -4;
				}
				
				// 如果没有数据，不断地请求数据包
				SendByte(CREQ);
				continue;
			}
			
			// rst == 0  /////////////////////////
			// 错误清零
			errors = 0;
			
			// 执行解析数据操作
			switch(packetLength){
				case -1: // 发送者通过协议告知发送中断 Ctrl + C
					USART1_send_string("User Cancel\n");
					SendByte(ACK);
					return 0;
				case 0: // 文件传输完毕
					if(fileDone == 0){
						SendByte(NAK);
						fileDone = 1;
						USART1_send_string("fileDone NAK\n");
					}else if(fileDone == 1){
						SendByte(ACK);
						fileDone = 2;
						USART1_send_string("fileDone ACK\n");
					}
					break;
				default:
					if(packetsReceived == 0){ // 第一个包
						// 1. 头包：文件名+数据长度
						if(packetData[PACKET_HEADER] != 0){
							// 文件开始
							// 读取固件名和固件大小
							getFileInfo(packetData, &size);
							
							// 校验固件大小
							if(size > FLASH_APP_SIZE){
								// 超出Flash的最大可用空间
								SendByte(CA);
								SendByte(CA);
								return -1;
							}
							// 从指定位置，擦除Flash，擦除size个大小的空间 0x08004000->16KB
							FlashErase(flashDestination, size);
							
							// 回复ACK和C
							SendByte(ACK);
							SendByte(CREQ);
						}else {
							// 会话结束(遇到新文件的第一个包，但是第4个字节是0x00)
							// 传输全部完毕：文件名为空内容，遇到会话结束的标记了
							/* Filename packet is empty, end session */
							SendByte(ACK);
							sessionDone = 1;
							fileDone = 2;
						}
					}else {
						// 2. 其他：解析数据写入到Flash
						// 将packetData的数据部分(跳过3个字节)拷贝到缓存里
						memcpy(bufPtr, packetData + PACKET_HEADER, packetLength);
						// 将bufPtr缓存里的数据写入到Flash对应位置
						FlashWrite(flashDestination, bufPtr, packetLength);
						// 将目标Flash地址往后移动
						flashDestination += packetLength;
						// 回复ACK
						SendByte(ACK);
					}
					
					// 数据包个数+=1
					packetsReceived ++;
					sessionBegin = 1;
					break;
			}
			
			if(fileDone > 1){
				break;
			}
		}
		
		if(sessionDone > 0){
			break;
		}
	}
	
//	sprintf(str_buf, "end, size: %d\n", size);
//	USART1_send_string(str_buf);
	
	return size;
}


void UpdateApp(void)
{
	uint8_t strBuffer[10] = "";
	int32_t imageSize = 0;

	printf("Waiting for the file to be sent ... (press 'a' to abort)\n\r");
	imageSize = YmodemReceive(g_packetBuffer);
	
	delay_1ms(50); // 需要等待一会，否则securecrt不能显示下面的打印信息

	sprintf(str_buf, "imageSize: %d\n", imageSize);
	USART1_send_string(str_buf);
    // 根据返回值，判定升级结果
	if (imageSize > 0)
	{
		printf("\r\n Programming Completed Successfully!\n\r");
//		Int2Str(strBuffer, imageSize);
		printf("[ Name: %s ,imageSize: %d Bytes]\r\n", g_imageName, imageSize);
	} 
	else if (imageSize == -1)
	{	// 固件过大，超出可用空间，无法下载
		printf("\r\nThe image size is higher than the allowed space memory!\n\r");
	}
	else if (imageSize == -2)
	{	// 固件校验失败
		printf("\r\nVerification failed!\n\r");
	}
	else if (imageSize == -3)
	{	// 上位机主动取消
		printf("\r\nAborted by user.\n\r");
	}
	else
	{	// 接收文件失败
		printf("\r\nFailed to receive the file!\n\r");
	}
}