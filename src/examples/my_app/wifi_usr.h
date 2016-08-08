#include <stdio.h>

extern uint8_t	node_id;
#ifdef __cplusplus
extern "C"
{
#endif
bool wifi_init(int fd);
void resetwifi(void);
uint8_t test_nlink(void);
uint8_t test_nready(void);
int poll_read(int fd, char* readbuf, uint8_t bytes_num);
void init_string(char *string);
void iniRD(void);
//bool EnterAT(int fd);
//void PollEnterAT(int fd);
int EnterAT(int fd);
int stop_return(int fd);
int ENTM(int fd);
int WMODE(int fd, char *para);
int WSSSID(int fd, char *para);
int WSKEY(int fd, char *para);
int NETP(int fd, char *para);
int ATZ(int fd);
void RELD(int fd);
void WSLK(int fd);
void WSCAN(int fd);
void TCPLK(int fd);
void TCPDIS(int fd);
int WANN(int fd);
	//void mag_cali(double);
#ifdef __cplusplus
}
#endif
