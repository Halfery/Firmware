#include "wifi_usr.h"
#include "iniRD.h"
//#include "receiver.h"
#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

#define  MAX_PATH 260
#define WIFI_INI "/fs/microsd/USRwifi.ini"

struct pollfd read_pollfd;
char netpara[5][50];

uint8_t	node_id = 0;
int wflag;
void resetwifi(void){
	//pull down the rst pin more than 300 milisec
	px4_arch_gpiowrite(GPIO_wifi_rst, 0);
	usleep(800);
	px4_arch_gpiowrite(GPIO_wifi_rst, 1);
	//usleep(1000000);
}

uint8_t test_nlink(void){
	uint8_t nlink;
	nlink = px4_arch_gpioread(GPIO_wifi_nlink);
	return nlink;
}

uint8_t test_nready(void){
	uint8_t nready;
	nready = px4_arch_gpioread(GPIO_wifi_nready);
	return nready;
}

int poll_read(int fd, char* readbuf, uint8_t bytes_num){
	int pollret;
	char tmpstring[100];
	init_string(tmpstring);
	read_pollfd.fd = fd;
	read_pollfd.events = POLLIN;
	pollret = poll(&read_pollfd, 1, 10000);
	if ((pollret == 1)&&(read_pollfd.revents == POLLIN)){
		read(fd, readbuf, bytes_num);
		//printf("%s ", readbuf);//debug only
		if ((readbuf[0] != '+')&&(readbuf[0] != 'A')) return 0; // successful
		while(strstr(readbuf, "\r\n\r\n") == NULL){
			strcat(tmpstring, readbuf);
			if (strstr(tmpstring, "\r\n\r\n") != NULL) {
				strcpy(readbuf, tmpstring);
				break;
			}
			read(fd, readbuf, bytes_num);
		}
		strcat(tmpstring, readbuf);
		strcpy(readbuf, tmpstring);
		return 0;
	}
	return -1;// something wrong
}

void init_string(char *string){
	uint8_t size = sizeof(string);
	for(int i = 0; i<size; i++) string[i] = 0;
}

void iniRD(void){
	FILE *fp,*fheadp;
	//char para[5][50];
	uint8_t index = 0;
	char stringbuf[100];
	char *tmpstring;
	uint8_t tokenlenth = 0;
	char token[100];
	for(int i=0; i<100; i++){token[i] = 0;}
	//char *fgetsreturn;
	fheadp = fopen("/fs/microsd/USRwifi.ini", "r");
	if (fheadp == NULL){
		printf("open file failed!\n");
	}
	fp = fheadp;
	while(fgets(stringbuf, 100, fp) != NULL){
		if (stringbuf[1] == '\0') continue;
		if (stringbuf[0] == '['){
			if (strlen(token) > 0) {
				tmpstring = strchr(token, '=');
				tmpstring++;
				strcpy(netpara[index], tmpstring);
				//printf("netpara %d is %s, strlen is %d\n", index, netpara[index], strlen(netpara[index]));
				tmpstring = strchr(netpara[index], '\n');
				*tmpstring = '\0';
				//printf("netpara %d is %s, strlen is %d\n", index, netpara[index], strlen(netpara[index]));
				index++;
			}
			tmpstring = &stringbuf[1];
			tokenlenth = strlen(tmpstring)-2;
			strncpy(token, tmpstring, tokenlenth);
			token[tokenlenth] = '\0';
			//printf("token is %s\n", token);
		}
		else{
			tmpstring = strchr(stringbuf, '=');
			if (strchr(token, '=') == NULL){
				//tmpstring = strchr(stringbuf, '=');
				strcat(token, tmpstring);
				tokenlenth = strlen(token);
				token[tokenlenth] = '\0';
				tokenlenth--;
				//printf("now token is %s\n", token);
				//printf("token lenth is %d\n", tokenlenth);
				//token[tokenlenth] = '\0';
			}
			else {
				++tmpstring;
				//printf("now tmpstring is %s\n", tmpstring);
				tokenlenth = strlen(token);
				token[tokenlenth-1]=',';
				strcat(token, tmpstring);
				tokenlenth = strlen(token);
				//token[tokenlenth]='\0';
			}
			//printf("token is %s\n", token);
		}
		//printf("stringbuf is %s, string length is %d\n", stringbuf, strlen(stringbuf));
	}
	tmpstring = strchr(token, '=');
	tmpstring++;
	strcpy(netpara[index], tmpstring);
	//printf("token is %s\n", para[index]);
	tmpstring = strchr(netpara[index], '\n');
	*tmpstring = '\0';
	fclose(fp);
}

bool wifi_init(int fd)
{
	iniRD();
	//char buf[MAX_PATH];
	//char *para,
	//char *pbuf;
	//pbuf = (char *)&buf;
	uint8_t nlink, nready, test_times;
	int wmode, wsssid, wsky, netp;
	if (fd <= 0)
		return false;

	//para = GetIniKeyString("SYS","id", WIFI_INI);
	node_id = atoi(netpara[4]);
	printf("ID is: %d\n", node_id);
/*********************************************************
* test the wifi module, and decide whether reset it or not
**********************************************************/
	nready = 1, test_times = 0;
	while(nready){
		nready = test_nready();
		if (test_times == 10){
			resetwifi();
		}
		test_times ++;
		usleep(100000);
	}
//start configuration,
	EnterAT(fd);
	usleep(100000);
	stop_return(fd);
	usleep(100000);

	wmode = WMODE(fd, netpara[0]);
	usleep(3000000);

	wsssid = WSSSID(fd, netpara[1]);
	usleep(200000);

	wsky = WSKEY(fd, netpara[2]);
	usleep(900000);

	netp = NETP(fd, netpara[3]);
	usleep(500000);

	if ((wmode&wsssid&wsky&netp)==0){//network parameter changed
		ATZ(fd);
		usleep(5000000);
	}
	else{
		ENTM(fd);
	}
	//ATZ(fd);
	//usleep(5000000);
//detecting wifi nlink pin, low active
	nlink = 1;
	while(nlink){
		nlink = test_nlink();
	}

	return true;
}

int EnterAT(int fd){//1
	char cmdbuf[] = "+++";
	char a = 'a';
	char readbuf[20];
	int pollret;
	char *tmpstring;
	init_string(readbuf);
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	usleep(10000);
	pollret = poll_read(fd, readbuf, 10);
	if (pollret == -1) return -1;//need reset wifi module
	printf("%s ", readbuf);//should print a
	init_string(readbuf);
	wflag = write(fd, &a, 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 10);
	if (pollret == -1) return -1;//need reset wifi module
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);//should print +ok
	//init_string(readbuf);
	return 0;
}

int stop_return(int fd){//2
	char cmdbuf[20]="AT+E\r";
	char readbuf[20];
	int pollret;
	char *tmpstring;
	init_string(readbuf);
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 10);
	//read(fd,readbuf,20);
	if (pollret == -1) return -1;//something must be wrong
	//tmpstring = strchr(readbuf, '\n');
	//*tmpstring = '\0';
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';	
	printf("%s\n", readbuf);
	//init_string(readbuf);
	return 0;//should print +ok
}

int WANN(int fd)					//查询或设置WAN设置
{
	char cmdbuf[100]="AT+WANN\r";
	char readbuf[100];
	//char tmp[50];
	int pollret;
	char *tmpstring;
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(10000);
	pollret = poll_read(fd, readbuf, 80);
	if (pollret == -1) return -1;//something must be wrong
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	//init_string(readbuf);
	//strncpy(tmp, (char *)readbuf, (strlen((char *)readbuf)));
	//printf("%s\n", tmp);
	return 0; // should not get there
}

int WMODE(int fd, char *para)//3			//wifi模块设置为STA模式
{
	char cmdbuf[100]="AT+WMODE";	//STA\r";
	char ret = '\r';
	//char ok[] = "+ok";
	char readbuf[100];
	char *pchar;
	int pollret;
	char *tmpstring;
	init_string(readbuf);
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	wflag = write(fd, &ret, 1);
	usleep(10000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	//printf("readbuf lenth is %d \n",strlen(readbuf));
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	//printf("readbuf lenth is %d \n",strlen(readbuf));
	printf("%s\n", readbuf);
	pchar = strstr((char *)readbuf, para);
	if (pchar != NULL) return 1; // network parameter unchanged
	init_string(readbuf);
	strcat(cmdbuf,"=");
	strcat(cmdbuf,para);
	strcat(cmdbuf,"\r");
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(10000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	//init_string(readbuf);
	return 0;
/*
	pchar = strstr((char *)readbuf, ok);
	if (pchar != NULL){
		printf("%s\n", ok);
		return 0;
	}
	if(wflag > 0)
	{
		printf("WIFI SET: %s\n",cmdbuf);
	}
*/
}

int WSSSID(int fd, char *para)//4			//查询或设置wifi模块的SSSID
{
	char cmdbuf[100]="AT+WSSSID";	//test\r";
	char ret = '\r';
	char readbuf[100];
	init_string(readbuf);
	char *pchar;
	int pollret;
	char *tmpstring;
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	wflag = write(fd, &ret, 1);
	usleep(1000000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	pchar = strstr((char *)readbuf, para);
	if (pchar != NULL){
		pchar = strchr(readbuf, '=');
		pchar++;
		if (strcmp(pchar, para) == 0) return 1;  //*para isn't substring of *pchar and unchanged
	}
	init_string(readbuf);
	strcat(cmdbuf,"=");
	strcat(cmdbuf,para);
	strcat(cmdbuf,"\r");
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s", readbuf);
	return 0;
}

int WSKEY(int fd, char *para)					//STA模式下查询或设置wifi模块的加密和连接密码
{
	//printf("enter wskey");
	char cmdbuf[100]="AT+WSKEY";	//WPA2PSK,AES,abc123456\r";
	char ret = '\r';
	char *tmpstring;
	char readbuf[100];
	char *pchar;
	int pollret;
	init_string(readbuf);
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	wflag = write(fd, &ret, 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	//usleep(400000);
	//read(fd,readbuf,50);
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	//strncpy(tmp, (char *)readbuf, (strlen((char *)readbuf)));
	//printf("1  %d\n", strlen(tmp));
	//printf("2  %d\n", strlen(para));
	printf("%s\n", readbuf);
	pchar = strstr(readbuf, para);
	if (pchar != NULL) return 1;/// network parameter unchanged
	init_string(readbuf);
	strcat(cmdbuf,"=");
	strcat(cmdbuf,para);
	strcat(cmdbuf,"\r");
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	//usleep(400000);	
	//read(fd,readbuf,50);
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	return 0;//should not get there
}

int NETP(int fd, char *para)					//设置要连接到的内网服务器IP和端口号
{
	//printf("enter netp \n");
	//printf("para is %s\n", para);
	char cmdbuf[100]="AT+NETP";	//TCP,CLIENT,8899,192.168.1.101\r";
	char ret = '\r';
	char readbuf[100];
	char *pchar;
	char *tmpstring;
	int pollret;
	init_string(readbuf);
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	wflag = write(fd, &ret, 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	//printf("readbuf length is %d\n", strlen(readbuf));
	//printf("netp is %s\n", readbuf);
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	pchar = strstr(readbuf, para);
	if (pchar != NULL) return 1;// network parameter unchanged
	init_string(readbuf);
	strcat(cmdbuf,"=");
	strcat(cmdbuf,para);
	strcat(cmdbuf,"\r");
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(100000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong
	tmpstring = strchr(readbuf, '\r');
	*tmpstring = '\0';
	printf("%s\n", readbuf);
	return 0;

}

int ATZ(int fd)					//wifi模块重启
{
	char cmdbuf[100]="AT+Z\r";
	char readbuf[100];
	char tmp[50];
	int pollret;
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf));
	usleep(10000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong	
	//read(fd,readbuf,50);
	//printf("1  %d\n", strlen((char *)readbuf));
	strncpy(tmp, (char *)readbuf, (strlen((char *)readbuf)));
	//printf("1  %d\n", strlen(tmp));
	//printf("2  %d\n", strlen(para));
	printf("ATZ%s\n", tmp);	
	init_string(readbuf);
	return 0;

	if(wflag > 0)
	{
		//printf("%d\n", strlen((char*)cmdbuf));
		printf("WIFI SET: %s\n",cmdbuf);
	}
	//usleep(7000000);//手册说明重启需要6s
}

int ENTM(int fd)		//进入透传模式
{
	char cmdbuf[100]="AT+ENTM\r";
	char readbuf[100];
	char tmp[50];
	int pollret;
	wflag = write(fd, cmdbuf, strlen((char*)cmdbuf) + 1);
	usleep(10000);
	pollret = poll_read(fd, readbuf, 50);
	if (pollret == -1) return -1;//something must be wrong	
	//read(fd,readbuf,50);
	strncpy(tmp, (char *)readbuf, (strlen((char *)readbuf)));
	printf("ENTM%s\n", tmp);
	init_string(readbuf);
	return 0;	
	if(wflag > 0 )
	{
		printf("WIFI SET: %s\n",cmdbuf);
	}
}


