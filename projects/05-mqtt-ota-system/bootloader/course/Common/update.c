#include "update.h"
#include "wifi_drv.h"
#include "store_app.h"
#include "systick.h"
#include "flash_drv.h"


uint16_t CalcCrc16(const uint8_t *data, uint16_t length)
{
		uint16_t polynomial = 0xA001;			 // CRC16-IBM 多项式 0xA001 0x8005
    uint16_t crc = 0x0000;             //  阿里云的是0
    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t) data[i]; // 直接 XOR 低字节，因为多项式是反转的
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x0001)              // 检查最低位
                crc = (crc >> 1) ^ polynomial; // 右移并应用多项式
            else
                crc >>= 1; // 只右移
        }
    }
    return crc;
}


typedef struct {

	/* 要发送的AT指令 */
	char *cmd;
	/* 期望得到的应答数据，返回结果包含该字符内容，即判定成功*/
	char *rsp;
	/* 应答最大时间（超时时间）*/
	uint32_t timeoutMs;
} AtCmdInfo_t;

// 检查任务的所有指令和响应数组
static AtCmdInfo_t g_checkWifiModuleCmdTable[] = {
	// 软复位
	{"AT+RST\r\n", "ready", 3000},
	// 纯等待
	{NULL, "XXX", 1000}, // 非阻塞式的等待1000ms
	{ // 关闭回显
		.cmd = "ATE0\r\n",
		.rsp = "OK",
		.timeoutMs = 500,
	},
	{	// 设置为Sta模式
		.cmd = "AT+CWMODE=1\r\n",
		.rsp = "OK",
		.timeoutMs = 1000,
	},
	
};

typedef enum {
	AT_RST, 			// 0
	AT_RST_DELAY,	// 1
	AT_E0,				// 2
	AT_CWMODE_1		// 3
} AtCheckModuleCmdType;

// 检查Wifi模块状态任务
WifiCommState_t CheckWifiModuleWork(){
	static AtCheckModuleCmdType cmdType = AT_RST;
	static uint8_t retryCount = 0;
	WifiCommState_t commState;
	
	switch(cmdType){
		case AT_RST:
			commState = AtCmdHandle(
							g_checkWifiModuleCmdTable[AT_RST].cmd, 
							g_checkWifiModuleCmdTable[AT_RST].rsp, 
							g_checkWifiModuleCmdTable[AT_RST].timeoutMs);
		
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				// 更新状态
				cmdType = AT_RST_DELAY;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		case AT_RST_DELAY:
			commState = AtCmdHandle(
							g_checkWifiModuleCmdTable[AT_RST_DELAY].cmd, 
							g_checkWifiModuleCmdTable[AT_RST_DELAY].rsp, 
							g_checkWifiModuleCmdTable[AT_RST_DELAY].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				ClearRecvWifiStr();
				// 更新状态
				cmdType = AT_E0;
			}else {
				// 更新状态
				cmdType = AT_E0;
			}
			break;
			
		case AT_E0:
			commState = AtCmdHandle(
							g_checkWifiModuleCmdTable[AT_E0].cmd, 
							g_checkWifiModuleCmdTable[AT_E0].rsp, 
							g_checkWifiModuleCmdTable[AT_E0].timeoutMs);
		
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				// 更新状态
				cmdType = AT_CWMODE_1;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
			
		case AT_CWMODE_1:
			commState = AtCmdHandle(
							g_checkWifiModuleCmdTable[AT_CWMODE_1].cmd, 
							g_checkWifiModuleCmdTable[AT_CWMODE_1].rsp, 
							g_checkWifiModuleCmdTable[AT_CWMODE_1].timeoutMs);
		
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				// 更新状态为默认重置状态
				cmdType = AT_RST;
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		default:
			break;
	}
	
	return WIFI_COMM_WAIT;
}
// 连接指定Wifi的所有指令和响应数组
static AtCmdInfo_t g_connectWifiCmdTable[] = {
	// 连接指定Wifi
	{
		.cmd="AT+CWJAP=\"%s\",\"%s\"\r\n", 
		.rsp="GOT IP", 
		.timeoutMs = 5000
	},
};

typedef enum {
	AT_CWJAP_SSID_PWD, 			// 0
} AtWifiConnectCmdType;

#define WIFI_SSID		"YOUR_WIFI_SSID"
#define WIFI_PWD		"YOUR_WIFI_PASSWORD"

// 检查Wifi模块状态任务
WifiCommState_t CheckWifiConnect(){
	static AtWifiConnectCmdType cmdType = AT_CWJAP_SSID_PWD;
	static uint8_t retryCount = 0;
	
	WifiCommState_t commState;
	char cmdStrBuf[256];
	
	switch(cmdType){
		case AT_CWJAP_SSID_PWD:
			sprintf(cmdStrBuf, g_connectWifiCmdTable[AT_CWJAP_SSID_PWD].cmd, WIFI_SSID, WIFI_PWD);
			commState = AtCmdHandle(cmdStrBuf, 
					g_connectWifiCmdTable[AT_CWJAP_SSID_PWD].rsp,
					g_connectWifiCmdTable[AT_CWJAP_SSID_PWD].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		default:
			break;
	}
	
	return WIFI_COMM_WAIT;
}
/*****
字符串 \\, 第二个\是给AT指令用的，第一个\是给编译器用的，加了第一个\,才能保留第二个斜线\
***/
const static char g_mqttClientId[]  = "YOUR_MQTT_CLIENT_ID";
const static char g_mqttUserName[]  = "YOUR_MQTT_USERNAME";
const static char g_mqttPassword[]  = "YOUR_MQTT_PASSWORD";
const static char g_mqttUrl[] 		  = "YOUR_MQTT_ENDPOINT";
const static char g_mqttSubUpgrade[]= "YOUR_OTA_UPGRADE_TOPIC";
const static char g_mqttSubDownload[]  = "YOUR_OTA_DOWNLOAD_TOPIC";

static AtCmdInfo_t g_connectMqttCmdTable[] = {
	{	// 本地配置连接信息
		.cmd = "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
		.rsp = "OK",
		.timeoutMs = 300,
	},
	{ // 连接MQTT服务器
		.cmd = "AT+MQTTCONN=0,\"%s\",1883,1\r\n",
		.rsp = "OK",
		.timeoutMs = 2000,
	},
	{	// 订阅OTA升级消息（如果服务器不主动推送，则必须订阅该地址才能拿到最新OTA固件信息）
		.cmd = "AT+MQTTSUB=0,\"%s\",0\r\n",
		.rsp = "OK",
		.timeoutMs = 1000,
	},
	{
    .cmd = "AT+MQTTSUB=0,\"%s\",0\r\n",
    .rsp = "OK",
    .timeoutMs = 1000,
  },
};

typedef enum
{
  AT_MQTTUSERCFG = 0,
  AT_MQTTCONN,
  AT_MQTTSUB_UPGRADE, // subscribe UPGRADE
  AT_MQTTSUB_DOWNLOAD, // subscribe Download
} AtConnectMqttCmdType;

// 连接MQTT服务器
WifiCommState_t ConnectMqttServer(void){

	static AtConnectMqttCmdType cmdType = AT_MQTTUSERCFG;
	static uint8_t retryCount = 0;
	WifiCommState_t commState;
	char cmdStrBuf[256];
	
	switch(cmdType){
		case AT_MQTTUSERCFG:
			// 格式化字符串
			sprintf(cmdStrBuf, g_connectMqttCmdTable[AT_MQTTUSERCFG].cmd, 
						g_mqttClientId, g_mqttUserName, g_mqttPassword);
			// 执行并等待
			commState = AtCmdHandle(cmdStrBuf, 
					g_connectMqttCmdTable[AT_MQTTUSERCFG].rsp,
					g_connectMqttCmdTable[AT_MQTTUSERCFG].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				cmdType = AT_MQTTCONN; // 状态流转
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		case AT_MQTTCONN:
			// 格式化字符串
			sprintf(cmdStrBuf, g_connectMqttCmdTable[AT_MQTTCONN].cmd, g_mqttUrl);
			// 执行并等待
			commState = AtCmdHandle(cmdStrBuf, 
					g_connectMqttCmdTable[AT_MQTTCONN].rsp,
					g_connectMqttCmdTable[AT_MQTTCONN].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				cmdType = AT_MQTTSUB_UPGRADE; // 状态流转
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		case AT_MQTTSUB_UPGRADE:
			
			// 格式化字符串
			sprintf(cmdStrBuf, g_connectMqttCmdTable[AT_MQTTSUB_UPGRADE].cmd, g_mqttSubUpgrade);
			// 执行并等待
			commState = AtCmdHandle(cmdStrBuf, 
					g_connectMqttCmdTable[AT_MQTTSUB_UPGRADE].rsp,
					g_connectMqttCmdTable[AT_MQTTSUB_UPGRADE].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				cmdType = AT_MQTTSUB_DOWNLOAD; // 状态流转
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		case AT_MQTTSUB_DOWNLOAD:
			
			// 格式化字符串
			sprintf(cmdStrBuf, g_connectMqttCmdTable[AT_MQTTSUB_DOWNLOAD].cmd, g_mqttSubDownload);
			// 执行并等待
			commState = AtCmdHandle(cmdStrBuf, 
					g_connectMqttCmdTable[AT_MQTTSUB_DOWNLOAD].rsp,
					g_connectMqttCmdTable[AT_MQTTSUB_DOWNLOAD].timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				ClearRecvWifiStr();
				cmdType = AT_MQTTUSERCFG; // 状态流转到默认值
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				if(retryCount >= 3){
					retryCount = 0;
					return WIFI_COMM_FAIL;
				}
			}
			break;
		default:
			break;
	}
	
	return WIFI_COMM_WAIT;
}



#define DOWNLOAD_SPLIT_SIZE		256

static uint32_t g_SplitTotalNum = 0;	// 下载总次数
static uint32_t g_splitCurrNum = 1;		// 下载当前包位置

static uint32_t g_otaBinSize = 0;			// 固件总大小
static uint32_t g_otaStreamId = 0;
static char g_otaVersion[16] = {0};

void ParseVersionInfo(char* json){
	// 找到size大小 --------------------------------------
	char* new_str = strstr(json, "\"size\":");
	if(new_str == NULL) return;
//	printf("new_str: %s\n", new_str);
	// 得到size后的数据
	sscanf(new_str, "\"size\":%ud,", &g_otaBinSize);
	
	// 查找streamId	--------------------------------------
	new_str = strstr(new_str, "\"streamId\":");
	if(new_str == NULL) return;
	// 得到streamId后的数据
	sscanf(new_str, "\"streamId\":%ud,", &g_otaStreamId);

	// 查找version	--------------------------------------
	new_str = strstr(new_str, "\"version\":\"");
	if(new_str == NULL) return;
	// 得到version后的数据

	// 找到开始位置
	new_str += strlen("\"version\":\"");
	// 找到结束位置
	char* end_str = strstr(new_str, "\",");
	// sscanf(new_str, "\"version\":\"%[^\"]", g_otaVersion);
	
//	printf("new_str: %s\n", new_str);
	// 清空现有字符串内容
	memset(g_otaVersion, 0, sizeof(g_otaVersion));
	// 从new_str拷贝N个字符到g_otaVersion 
	strncpy(g_otaVersion, new_str, end_str - new_str);

	printf("ParsedInfo, size = %d\n", g_otaBinSize);
	printf("ParsedInfo, streamId = %d\n", g_otaStreamId);
	printf("ParsedInfo, version = %s\n", g_otaVersion);
}

const static char g_mqttPubDownload[]  = "YOUR_OTA_DOWNLOAD_REQUEST_TOPIC";

const static char g_mqttPubInform[]  = "YOUR_OTA_PROGRESS_TOPIC";

#define NOTIFI_FINISH	   0		// 设置0， 不进行完成通知

static AtCmdInfo_t g_commMqttCmdTable[] = {
	{
		.cmd = NULL,
		.rsp = (char *)g_mqttSubUpgrade,
		.timeoutMs = 10000,
	},
	{
		.cmd = "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"1\\\"\\,\\\"params\\\":{\\\"fileInfo\\\":{\\\"streamId\\\":%d\\,\\\"fileId\\\":1}\\,\\\"fileBlock\\\":{\\\"size\\\":%d\\,\\\"offset\\\":%d}}}\",0,0\r\n",
		.rsp = "download_reply",
		.timeoutMs = 5000,
	},
	{
		.cmd = "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"1\\\"\\,\\\"params\\\":{\\\"version\\\":\\\"%s\\\"}}\",0,0\r\n",
		.rsp = "OK",
		.timeoutMs = 500,
	}
};
typedef enum{

	AT_OTA_CHECK = 0,
	AT_OTA_DOWNLOAD,
	AT_OTA_FINISH,
//	AT_MQTT_PUB,		
	
} AtCommMqttCmdType;

// 与MQTT服务器通讯
WifiCommState_t OtaHandleDownloadFile(void){
	static AtCommMqttCmdType cmdType = AT_OTA_CHECK;
	static uint8_t retryCount = 0;
	static uint32_t s_splitDownloadSize = 0;
	WifiCommState_t commState;
	char cmdStrBuf[256];
	
	AtCmdInfo_t cmdInfo = g_commMqttCmdTable[cmdType];
	switch(cmdType){
		case AT_OTA_CHECK:
			// 执行并等待
			commState = AtCmdHandle(cmdInfo.cmd, cmdInfo.rsp, cmdInfo.timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				retryCount = 0;
				
				printf("OTA check success! \n"); // g_recvStrBuf
				ParseVersionInfo(g_recvStrBuf);
				ClearRecvWifiStr();
				// size: 16200  SPLIT: 256

				// 计算下载次数
				if(g_otaBinSize % DOWNLOAD_SPLIT_SIZE == 0){ // 正好整除
					g_SplitTotalNum = g_otaBinSize / DOWNLOAD_SPLIT_SIZE;
				}else {
					// 最后一包请求的数量，不是256
					g_SplitTotalNum = g_otaBinSize / DOWNLOAD_SPLIT_SIZE + 1;
				}
				// 要下载的字节数量
				s_splitDownloadSize = DOWNLOAD_SPLIT_SIZE;
				// 先清理目标区域Flash
				FlashErase(BACKUP_ADDR_IN_FLASH, g_otaBinSize);
				g_splitCurrNum = 1;
				
				// 流转到下载状态
				cmdType = AT_OTA_DOWNLOAD;
				
			}else if(commState == WIFI_COMM_FAIL){
				cmdType = AT_OTA_CHECK;
				printf("OTA check fail!\n");
				return WIFI_COMM_FAIL;
			}
			break;
		case AT_OTA_DOWNLOAD:
			/**************************************
			{
					"id": "1",
					"params": {
							"fileInfo": {
									"streamId": 22204,			// param 1
									"fileId": 1
							},
							"fileBlock": {
									"size": 256,						// param 2
									"offset": 0							// param 3   [0, 256, 512 ...
							}
					}
			}
			
			size: 16200  SPLIT: 256
			***************************************/
			sprintf(cmdStrBuf, cmdInfo.cmd, g_mqttPubDownload, g_otaStreamId, 
							s_splitDownloadSize,  									   // fileBlock.size
							(g_splitCurrNum - 1) * DOWNLOAD_SPLIT_SIZE // fileBlock.offset
			);	
		
			commState = AtCmdHandle(cmdStrBuf, cmdInfo.rsp, cmdInfo.timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				
				// 总长度 = N字节JSON+256字节数据+2字节校验码+2字节\r\n
				
				// 一、进行数据CRC校验(判断相等)
				// 1) 先计算实际值: 找到字节开始位置
				uint8_t * start_pos = (uint8_t *)&g_recvStrBuf[g_recvStrLen - s_splitDownloadSize - 4];
				uint16_t actual_crc16 = CalcCrc16(start_pos, s_splitDownloadSize); // 实际值
				
				// 2) 再取出期望值
				uint16_t except_crc16 = *(uint16_t *)&g_recvStrBuf[g_recvStrLen - 4];
				
				// 3) 判断相等
				if(actual_crc16 != except_crc16){
					retryCount++;
					printf("**************CRC Error**************, acutal: 0x%X except: 0x%X\n", actual_crc16, except_crc16);
					
					if(retryCount == 3){
						retryCount = 0;
						cmdType = AT_OTA_CHECK;
						return WIFI_COMM_FAIL;
					}
					
					break; // 校验失败
				}
				
				// 二、校验通过：将数据写入到Backup对应区域
//				printf("Flash to backup: %d\n", g_splitCurrNum);
				// 计算写入的Flash开始位置
				uint32_t flash_start = BACKUP_ADDR_IN_FLASH + (g_splitCurrNum - 1) * DOWNLOAD_SPLIT_SIZE;
				FlashWrite(flash_start, start_pos, s_splitDownloadSize);
				
				ClearRecvWifiStr();
				retryCount = 0;
				
				// 三、下载下一个包：计算要下载的字节数
				g_splitCurrNum++;
				if(g_splitCurrNum == g_SplitTotalNum){ // 最后一个包的数据长度
					if(g_otaBinSize % DOWNLOAD_SPLIT_SIZE == 0){ // 正好整除
						s_splitDownloadSize = g_otaBinSize / DOWNLOAD_SPLIT_SIZE;
					}else {
						// 最后一包请求的数量，不是256
						s_splitDownloadSize = g_otaBinSize % DOWNLOAD_SPLIT_SIZE;
					}
				}else if(g_splitCurrNum < g_SplitTotalNum){ // 其他包
					s_splitDownloadSize = DOWNLOAD_SPLIT_SIZE;
				}else { // > 总数
					// 如果个数够了，结束
					
					
					ClearRecvWifiStr();
#if NOTIFI_FINISH
					cmdType = AT_OTA_FINISH;
#else
					printf("OTA Download success: %d!!!\n", g_splitCurrNum);
					cmdType = AT_OTA_CHECK;
					return WIFI_COMM_OK;
#endif
				}
				
				
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
				retryCount++;
				
				printf("OTA Download fail: %d\n", retryCount);
				
				if(retryCount >= 6){
					retryCount = 0;
					cmdType = AT_OTA_CHECK;
					return WIFI_COMM_FAIL;
				}
			}
			
			break;
		case AT_OTA_FINISH:
#if NOTIFI_FINISH
			sprintf(cmdStrBuf, cmdInfo.cmd, g_mqttPubInform, g_otaVersion);	
		
			commState = AtCmdHandle(cmdStrBuf, cmdInfo.rsp, cmdInfo.timeoutMs);
			if(commState == WIFI_COMM_OK){	// 成功，流传到下一个子状态
				ClearRecvWifiStr();
				cmdType = AT_OTA_CHECK;				
				printf("OTA Download FINISH!!!!!\n");
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				cmdType = AT_OTA_CHECK;			
				return WIFI_COMM_FAIL;	
			}
#endif
			break;
		default:
			break;
	}
	
	
	return WIFI_COMM_WAIT;
}

typedef enum
{
  CHECK_WIFI_MODULE,		// 检查设置Wifi模块,并设置模式为Sta
  CHECK_WIFI_CONNECT,		// 连接指定Wifi
  CONNECT_MQTT_SERVER,	// 连接指定MQTT服务器
  OTA_HANDLE,		  			// 下载OTA固件
  HWRESET_WIFI_MODULE,	// 硬件模块重置
  WIFI_MODULE_ERROR,
} WifiWorkState_t;

// 奥卡姆剃刀

WifiCommState_t OtaDownloadHandle(void){
	static WifiWorkState_t workState = CHECK_WIFI_MODULE;
	static uint8_t hwresetCnt = 0;
	
	WifiCommState_t commState; // WAIT, OK, FAIL
	
	switch (workState)
  {
  	case CHECK_WIFI_MODULE:		// 检查设置Wifi模块
			commState = CheckWifiModuleWork(); 
			if(commState == WIFI_COMM_OK){
				workState = CHECK_WIFI_CONNECT;
			}else if(commState == WIFI_COMM_FAIL){
				workState = HWRESET_WIFI_MODULE;
			}
  		break;
  	case CHECK_WIFI_CONNECT:	// 连接指定Wifi
			commState = CheckWifiConnect(); 
			if(commState == WIFI_COMM_OK){
				workState = CONNECT_MQTT_SERVER;
			}else if(commState == WIFI_COMM_FAIL){
				workState = CHECK_WIFI_MODULE;
			}
  		break;
  	case CONNECT_MQTT_SERVER:	// 连接指定MQTT服务器
//			printf("连接MQTT\n");
			commState = ConnectMqttServer();
		
			if(commState == WIFI_COMM_OK){
				workState = OTA_HANDLE;
			}else if(commState == WIFI_COMM_FAIL){
				workState = CHECK_WIFI_MODULE;
			}
		
  		break;
			
		case OTA_HANDLE:
			commState = OtaHandleDownloadFile();
			if(commState == WIFI_COMM_OK){
				workState = CHECK_WIFI_MODULE;
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				workState = CHECK_WIFI_MODULE;
				return WIFI_COMM_FAIL;
			}
		
			break;
  	case HWRESET_WIFI_MODULE:	// 硬件模块重置
			
			if(hwresetCnt == 0){
				hwresetCnt++;
				
				HwResetWifiModule();	// 如果AT基本命令失败，Reset硬重置一次
				delay_1ms(1000);
				workState = CHECK_WIFI_MODULE;
			}else {
				printf("Wifi Module error!\n");
				workState = WIFI_MODULE_ERROR;
			}
  		break;
  	default:
  		break;
  }
	
	return WIFI_COMM_WAIT;
}

// 这里很危险，提醒用户不要断电
// 1Kb
uint8_t g_flashReadBuf[FLASH_PAGE_SIZE];
#define ERASE_SIZE_MAX		((16 + 16 + 64 + 128) * 1024)
bool OtaUpdateFlashHandle(){
	
	// 1. 开始进行固件覆盖
	printf("1. 清理旧固件\n");
	FlashErase(APP_ADDR_IN_FLASH, ERASE_SIZE_MAX - 1);
	
	// 2. 读取Backup固件
	printf("2. 开始固件覆盖\n");
	
	uint16_t count = g_otaBinSize / FLASH_PAGE_SIZE + 1;
	// 3. 循环继续Backup读取，App写入
	for(uint16_t i = 0; i < count; i++){
		// 读Backup
		FlashRead(BACKUP_ADDR_IN_FLASH + i * FLASH_PAGE_SIZE, g_flashReadBuf, FLASH_PAGE_SIZE);
		// 写App
		bool rst = FlashWrite(APP_ADDR_IN_FLASH + i * FLASH_PAGE_SIZE, g_flashReadBuf, FLASH_PAGE_SIZE);
			
		printf("Write flash [%d] -> %s\n", i, rst ? "suceess" : "fail");
		
		if(!rst) return false;
	}
	
	return true;
}

uint8_t UpdateApp(void){

	WifiCommState_t commState;
	
	commState = OtaDownloadHandle();
	switch (commState)
  {
  	case WIFI_COMM_WAIT:
			return 0;
  	case WIFI_COMM_OK:{
			// 把Backup区的新固件写入到App区
			bool success = OtaUpdateFlashHandle();
			// 存储新的版本号
			SetSoftwareVersionParam(g_otaVersion);
			// 清理升级标记
			ClearUpdateVerFlag();
			printf("OtaUpdate success!\n");
			
			return 1;
		}
  	case WIFI_COMM_FAIL:
			// 清理升级标记
			ClearUpdateVerFlag();
			printf("OtaUpdate fail!\n");
		
			return 2;
  	default:
  		break;
  }
	
	return 0;		// 等待
}

// 获取当前进度 [0,100]
uint8_t GetUpdateProgress(void){

	return g_splitCurrNum * 100 / g_SplitTotalNum; // 3200 / 64
}

bool GetNewVersionInfo(char *version, uint32_t *binSize)
{
  char strBuf[10] = {0};
  if (g_otaBinSize == 0 || version == NULL)
  {
    return false;
  }
  GetSoftwareVersionParam(strBuf);
  if (strcmp(strBuf, g_otaVersion) == 0)
  {
    return false;
  }
  strcpy(version, g_otaVersion);
  *binSize = g_otaBinSize;
  return true;
}