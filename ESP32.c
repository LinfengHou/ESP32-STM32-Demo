#include "ESP32.h"

static uint8_t esp32_write_buf[MAX_BUF_SIZE];

UART_HandleTypeDef huart2; //ESP32ģ�鴮��

UART_HandleTypeDef huart1; //Debug����

uint8_t  USART2_RX_BUF[USART2_MAX_RECV_LEN];

static void MX_USART2_UART_Init(void);

static void MX_USART1_UART_Init(void);

/* ============================================================
��������esp32_check_ack
���ã����ִ�н���Ƿ���ȷ
�βΣ���Ҫ����Ԥ�ڽ��
����ֵ������0��ΪԤ�ڽ��������1Ϊ�ﵽԤ��
=============================================================== */

uint8_t* esp32_check_ack(char *str)
{
		char *strx=0;
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
		return (uint8_t*)strx;
}

/* ============================================================
��������esp32_send_cmd
���ã�����ATָ�����ִ�н��
�βΣ�����ָ�Ԥ�ڽ�����ȴ�ʱ��
����ֵ������0Ϊ�������ݳ�������1Ϊ����
=============================================================== */

int esp32_send_cmd(uint8_t *cmd,char *ack,uint16_t waittime)
{
		int res = 0;
		int len = strlen((const char*)cmd);
		int acklen = strlen((const char*)ack);
		memset(USART2_RX_BUF,0,USART2_MAX_RECV_LEN);
		HAL_UART_Transmit(&huart2,cmd,len,200);
		HAL_UART_Receive(&huart2,USART2_RX_BUF,USART2_MAX_RECV_LEN,waittime);
		if(esp32_check_ack(ack))res = 1;
		return res;
} 	

/* ============================================================
��������esp32_send_data
���ã��������ʹ�������
�βΣ���������
����ֵ������0Ϊ�������ݳ�������1Ϊ����
=============================================================== */

int esp32_send_data(uint8_t *data)
{
		int len = strlen((const char*)data);
		HAL_UART_Transmit(&huart2, data ,len ,200);
		return 1;
}

/* ============================================================
��������esp32_debug_printf
���ã����ʹ���Debug����
�βΣ���������
����ֵ������0Ϊ�������ݳ�������1Ϊ����
=============================================================== */

int esp32_debug_printf(char *data)
{
		int len = strlen((const char*)data);
		HAL_UART_Transmit(&huart1,(uint8_t*)data ,len ,200);
		return 1;
}

/* ============================================================
��������esp32_mod_init()
���ã����ģ���Ƿ��ʼ�����
�βΣ���
����ֵ������1��ʾģ��δ��ʼ���������������0Ϊ������
=============================================================== */

int esp32_mod_init(){
	  char *AT = "AT\r\n";
    memset(esp32_write_buf, 0, 18);
	  sprintf((char*)esp32_write_buf, "%s", AT);
	  if(esp32_send_cmd(esp32_write_buf, "OK" ,1000)){
			  esp32_debug_printf("������ɣ�\r\n");
				return 0;
		}else{
			  esp32_debug_printf("���������...\r\n");
				return 1;
		}
}

/* ============================================================
��������esp_set_wifi_mode
���ã�����WIFI����ģʽ
�βΣ�int�ͣ�Ϊѡ��ģʽ��0Ϊ��wifiģʽ���ر�wifi��1ΪStationģʽ
2ΪSoftAPģʽ��3ΪSoftAP+Stationģʽ
����ֵ������1Ϊ�������ݳ�������0Ϊ����
=============================================================== */

int esp_set_wifi_mode(int mode)
{
    char *AT = "AT+CWMODE=";
    memset(esp32_write_buf, 0, 18);
    if (mode == 0 || mode == 1 || mode == 2 || mode == 3)
    {
        sprintf((char*)esp32_write_buf, "%s%d%s", AT, mode, "\r\n");
    }
    else
    { 
			  esp32_debug_printf("WIFIģʽ����ʧ�ܣ���������\r\n");
        return 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
		{      
			  esp32_debug_printf("WIFIģʽ���óɹ���\r\n");
        return 0; 
    }else
    {
			  esp32_debug_printf("WIFIģʽ����ʧ�ܣ�\r\n");
        return 1;
    }
}

/* ============================================================
��������esp_set_wifi_connect
���ã�����WIFI���Ӳ���
�βΣ�ssidΪĿ��AP��SSID��pwdΪĿ��AP���룬macΪĿ��AP�����ַ��macenΪ�Ƿ�����mac��ַ
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_connect(char *ssid,char *pwd, char *mac ,int macen)
{
    char *AT = "AT+CWJAP=";
    memset(esp32_write_buf, 0, 64);
	  if(macen){
				sprintf((char*)esp32_write_buf, "%s\"%s\",\"%s\",\"%s\"%s", AT, ssid, pwd, mac, "\r\n");
		}else
		{
				sprintf((char*)esp32_write_buf, "%s\"%s\",\"%s\"%s", AT, ssid, pwd , "\r\n");
		}
    if(esp32_send_cmd(esp32_write_buf, "WIFI CONNECTED" ,5000))
    {        
        esp32_debug_printf("WIFI���ӳɹ���\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("WIFI����ʧ�ܣ�\r\n");
        return 1;
    }
}

/* ============================================================
��������esp_set_wifi_disconnect
���ã��Ͽ���wifi������
�βΣ���
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_disconnect()
{
    char *AT = "AT+CWQAP\r\n";
    if(esp32_send_cmd((uint8_t *)AT, "OK" ,1000))
    {           
        esp32_debug_printf("WIFI�ѶϿ���\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("WIFI�Ͽ�ʧ�ܣ�\r\n");
        return 1;
    }
}

/* ============================================================
��������esp_set_wifi_autocon
���ã�����wifi�Ƿ��ϵ��Զ�����
�βΣ�0���ϵ粻�Զ����ӡ�1���ϵ��Զ�����
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_autocon(int stat)
{
	  int res = 0;
    char *AT = "AT+CWAUTOCONN=";
    memset(esp32_write_buf, 0, 24);
    if (stat == 0 || stat == 1)
    {
        sprintf((char*)esp32_write_buf, "%s%d%s", AT, stat, "\r\n");
    }
    else
    { 
			  esp32_debug_printf("�Զ��ϵ���������ʧ�ܣ���������\r\n");
        res = 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
    {     
				esp32_debug_printf("�Զ��ϵ��������óɹ���\r\n");
    }else                                              
    {
			  esp32_debug_printf("�Զ��ϵ���������ʧ�ܣ�\r\n");
        res = 1;
    }
		return res;
}

/* ============================================================
��������esp_set_wifi_cipmode
���ã�����wifiģ�鴫��ģʽ
�βΣ�0Ϊ��ͨģʽ���䣬1Ϊ͸��ģʽ���䡣
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_cipmode(int mode)
{
    char *AT = "AT+CIPMODE=";
    memset(esp32_write_buf, 0, 24);
		if (mode == 0 || mode == 1)
    {
				sprintf((char*)esp32_write_buf, "%s%d%s", AT, mode, "\r\n");
		}else
    { 
			  esp32_debug_printf("����ʧ�ܣ���������\r\n");
        return 1;
    }
    if(esp32_send_cmd(esp32_write_buf, "OK" ,1000))
    {     
			  esp32_debug_printf("����ģʽ���óɹ���\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("����ģʽ����ʧ�ܣ�\r\n");
        return 1;
    }
}

/* ============================================================
��������esp_set_wifi_tcp
���ã��������������TCP����
�βΣ�1��������IP��ַ��2���˿ںš�
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_tcp(char* ip,int port)
{
    char *AT = "AT+CIPSTART=\"TCP\",\"";
    memset(esp32_write_buf, 0, 64);
    sprintf((char*)esp32_write_buf, "%s%s%s%d%s", AT, ip, "\",", port, "\r\n");
    if(esp32_send_cmd(esp32_write_buf, "CONNECT" ,1000))
    {     
			  esp32_debug_printf("TCP���ӽ����ɹ���\r\n");
        return 0;
    }else                                              
    {
			  esp32_debug_printf("TCP���ӽ���ʧ�ܣ�\r\n");
        return 1;
    }
}

/* ============================================================
��������esp_set_wifi_tcp
���ã�͸��ģʽ����
�βΣ�1Ϊ����͸��ģʽ��0Ϊ�˳�͸��ģʽ
����ֵ������1Ϊ���ó�������0Ϊ����
=============================================================== */

int esp_set_wifi_opmode(int mode)
{
    memset(esp32_write_buf, 0, 24);
		if (mode == 0 || mode == 1)
    {
				if(mode == 1)
				{
						sprintf((char*)esp32_write_buf, "%s", "AT+CIPSEND\r\n");
						if(esp32_send_cmd(esp32_write_buf, ">" ,1000))
						{     
								esp32_debug_printf("����͸��ģʽ�ɹ���\r\n");
								return 0;
						}else                                              
						{
								esp32_debug_printf("����͸��ģʽʧ�ܣ�\r\n");
								return 1;
						}
				}else{
						sprintf((char*)esp32_write_buf, "%s", "+++");
						esp32_send_data(esp32_write_buf);
						HAL_Delay(1000);
						esp32_debug_printf("���˳�͸��ģʽ��\r\n");
					  return 0;
				}
		}else
    { 
			  esp32_debug_printf("����ʧ�ܣ���������\r\n");
        return 1;
    }
   
}

/* ============================================================
��������esp_set_wifi_tcp
���ã�����TCP����
�βΣ�1��TCP���ݡ�2������
����ֵ������1Ϊ��������0Ϊ����
=============================================================== */

int esp_send_tcp_data(uint8_t *data,int len)
{
    char *AT = "AT+CIPSEND=";
    memset(esp32_write_buf, 0, 18);
    sprintf((char*)esp32_write_buf, "%s%d%s", AT, len, "\r\n");
    if(esp32_send_cmd(esp32_write_buf, ">" ,1000))
    {     
			  if(esp32_send_data(data))
				{
						esp32_debug_printf("TCP�����ѷ��ͣ�\r\n");
						return 0;
				}else{
						esp32_debug_printf("TCP���ݷ���ʧ�ܣ�\r\n");
						return 1;
				}
    }else                                              
    {
				esp32_debug_printf("TCP���ݷ���ʧ�ܣ�δ�����������\r\n");
				return 1;
    }
}

/* ============================================================
��������esp32_init()
���ã���⣬��ʼ��ģ�飬��������ѭ����⣬���debug
�βΣ���
����ֵ����
=============================================================== */

void esp32_init()
{
	  MX_USART2_UART_Init(); //��ʼ��ģ�鴮��
	  MX_USART1_UART_Init(); //��ʼ��Debug����
	  while(esp32_mod_init()); //ѭ�����ģ��������AT�Ƿ�ظ�OK��
}

/* ============================================================
��������MX_USART2_UART_Init()
���ã�HAL�⴮�ڳ�ʼ����������̬�������ڲ�����
�βΣ���
����ֵ����
=============================================================== */

static void MX_USART2_UART_Init(void)
{

		huart2.Instance = USART2;
		huart2.Init.BaudRate = 115200;
		huart2.Init.WordLength = UART_WORDLENGTH_8B;
		huart2.Init.StopBits = UART_STOPBITS_1;
		huart2.Init.Parity = UART_PARITY_NONE;
		huart2.Init.Mode = UART_MODE_TX_RX;
		huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart2.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart2) != HAL_OK)
		{
				_Error_Handler(__FILE__, __LINE__);
		}

}

/* ============================================================
��������MX_USART1_UART_Init()
���ã�HAL�⴮�ڳ�ʼ����������̬�������ڲ�����
�βΣ���
����ֵ����
=============================================================== */

static void MX_USART1_UART_Init(void)
{

		huart1.Instance = USART1;
		huart1.Init.BaudRate = 115200;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.Mode = UART_MODE_TX_RX;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.OverSampling = UART_OVERSAMPLING_16;
		if (HAL_UART_Init(&huart1) != HAL_OK)
		{
				_Error_Handler(__FILE__, __LINE__);
		}

}

