#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <stddef.h>

#define SERIAL_PORT "/dev/ttyAS5"
#define BAUDRATE B9600

extern int fd;


int serial_open(const char *port_path);
int serial_send(int fd, const char *data, size_t len);
int serial_recv(int fd, char *buf, size_t maxlen);
void serial_close(int fd);
int test_serial();
int test_serial_2();

typedef void (*serial_rx_callback_t)(const char *data, int len);

void serial_set_rx_callback(serial_rx_callback_t cb);
void *serial_rx_thread(void *arg);
int serial_start_rx(int fd);
void serial_stop_rx();

void my_rx_callback(const char *data, int len);
extern volatile int goto_music;
extern volatile int goto_music_pause;
extern volatile int goto_music_play;
extern volatile int goto_music_next;
extern volatile int goto_music_prev;
extern volatile int return_time_flag;
extern volatile int return_date_flag;
extern volatile int return_week_flag;
extern volatile int return_weather_flag;
extern volatile int goto_backup_flag;
extern volatile int goto_home_flag;
 

extern char time_str_uart[9];
extern char date_str_uart[20];
extern char week_str_uart[10];
extern char weather_str_uart[100];




#endif