#include "App.h"
#include "USART.h"
#include "wifi_drv.h"
#include "systick.h"

void App_Wifi_init(){
//	printf("App_Wifi_init\n");
	WifiModuleDrvInit();
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
		.timeoutMs = 500,
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
};

typedef enum
{
  AT_MQTTUSERCFG = 0,
  AT_MQTTCONN,
  AT_MQTTSUB_UPGRADE, // subscribe UPGRADE
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
				cmdType = AT_MQTTUSERCFG; // 状态流转
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




static uint32_t g_otaBinSize = 0;
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
	
	printf("new_str: %s\n", new_str);
	// 清空现有字符串内容
	memset(g_otaVersion, 0, sizeof(g_otaVersion));
	// 从new_str拷贝N个字符到g_otaVersion 
	strncpy(g_otaVersion, new_str, end_str - new_str);

	printf("ParsedInfo, size = %d\n", g_otaBinSize);
	printf("ParsedInfo, streamId = %d\n", g_otaStreamId);
	printf("ParsedInfo, version = %s\n", g_otaVersion);
}
static AtCmdInfo_t g_commMqttCmdTable[] = {
	{
		.cmd = NULL,
		.rsp = (char *)g_mqttSubUpgrade,
		.timeoutMs = 3000,
	},{
		.cmd = NULL,
		.rsp = "OK",
		.timeoutMs = 1000,
	}
};
typedef enum{

	AT_OTA_CHECK = 0,
	AT_MQTT_PUB,
	
} AtCommMqttCmdType;

// 与MQTT服务器通讯
WifiCommState_t CommMqttServer(void){
	static AtCommMqttCmdType cmdType = AT_OTA_CHECK;
	static uint8_t retryCount = 0;
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
				return WIFI_COMM_OK;
			}else if(commState == WIFI_COMM_FAIL){
				// 记录失败次数，失败超过3次，返回失败
//				retryCount++;
//				if(retryCount >= 3){
//					retryCount = 0;
//					return WIFI_COMM_FAIL;
//				}
				printf("OTA check fail!\n");
			}
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
  COMM_MQTT_SERVER,		  // 与MQTT服务器通讯
  HWRESET_WIFI_MODULE,	// 硬件模块重置
  WIFI_MODULE_ERROR,
} WifiWorkState_t;

WifiWorkState_t workState = CHECK_WIFI_MODULE;

void App_Wifi_task(){ // 10ms
//	printf("App_Wifi_task\n");
	
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
				workState = COMM_MQTT_SERVER;
			}else if(commState == WIFI_COMM_FAIL){
				workState = CHECK_WIFI_MODULE;
			}
		
  		break;
  	case COMM_MQTT_SERVER:		// 与MQTT服务器通讯
//			printf("与MQTT服务器通讯\n");
		
			commState = CommMqttServer();
			if(commState == WIFI_COMM_OK){
//				workState = COMM_MQTT_SERVER;
				// 可以跳转IDLE
			}else if(commState == WIFI_COMM_FAIL){
				workState = CHECK_WIFI_MODULE;
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
	// 如果检查任务失败WIFI_COMM_FAIL，进行硬件重置
	// 如果检查任务成功WIFI_COMM_OK，连接Wifi
	
}
#include "store_app.h"
bool App_Wifi_GetNewVersionInfo(char* new_version, uint32_t* bin_size){
	
	if(g_otaBinSize == 0 || new_version == NULL){
		// 无新固件
		return false;
	}
	
	char strBuf[10] = {0};
	// 检查现有版本strBuf和最新版本g_otaVersion是否相同 1.0
	GetSoftwareVersionParam(strBuf);
	if(strcmp(strBuf, g_otaVersion) == 0){
		// 相同，返回
		return false;
	}
	
	// 把新版本拷贝给 new_version
	strcpy(new_version, g_otaVersion);
	*bin_size = g_otaBinSize;
	
	return true;
}


