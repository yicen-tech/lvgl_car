#include "serial_comm.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>

#define SERIAL_PORT "/dev/ttyAS5"
#define BAUDRATE B9600

typedef void (*serial_rx_callback_t)(const char *data, int len);
static serial_rx_callback_t g_rx_cb = NULL;
static int g_serial_fd = -1;
static pthread_t g_rx_thread;
static int g_thread_running = 0;

// 打开并配置串口
int serial_open(const char *port_path) {
    int fd = open(port_path, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("无法打开串口");
        return -1;
    }

    struct termios options;
    tcgetattr(fd, &options);

    // 设置波特率
    cfsetispeed(&options, BAUDRATE);
    cfsetospeed(&options, BAUDRATE);

    // 8N1 模式
    options.c_cflag &= ~PARENB; // 无校验
    options.c_cflag &= ~CSTOPB; // 1位停止位
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     // 8位数据位

    options.c_cflag |= (CLOCAL | CREAD); // 本地连接，接收使能
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 原始输入
    options.c_oflag &= ~OPOST; // 原始输出

    tcsetattr(fd, TCSANOW, &options);

    return fd;
}

// 串口发送数据
int serial_send(int fd, const char *data, size_t len) {
    int n = write(fd, data, len);
    if (n < 0) {
        perror("串口发送失败");
        return -1;
    }
    return n;
}

// 串口接收数据
int serial_recv(int fd, char *buf, size_t maxlen) {
    int n = read(fd, buf, maxlen);
    if (n < 0) {
        perror("串口接收失败");
        return -1;
    }
    return n;
}

// 关闭串口
void serial_close(int fd) {
    close(fd);
}

// 测试主函数
int test_serial() {
    int fd = serial_open(SERIAL_PORT);
    if (fd < 0) {
        return -1;
    }

    // 发送数据
    const char *msg = "1";
    serial_send(fd, msg, strlen(msg));
    printf("发送数据: %s\n", msg);
    usleep(100000);

    // // 接收数据
    // char buf[256] = {0};
    // int n = serial_recv(fd, buf, sizeof(buf) - 1);
    // if (n > 0) {
    //     printf("收到数据: %s\n", buf);
    // }

    serial_close(fd);
    return 0;
}
// 测试主函数
int test_serial_2() {
    int fd = serial_open(SERIAL_PORT);
    if (fd < 0) {
        return -1;
    }

    // 发送数据
    const char *msg = "2";
    serial_send(fd, msg, strlen(msg));
    printf("发送数据: %s\n", msg);
    usleep(100000);
    
    // // 接收数据
    // char buf[256] = {0};
    // int n = serial_recv(fd, buf, sizeof(buf) - 1);
    // if (n > 0) {
    //     printf("收到数据: %s\n", buf);
    // }

    serial_close(fd);
    return 0;
}





void serial_set_rx_callback(serial_rx_callback_t cb) {
    g_rx_cb = cb;
}

// 串口接收线程
void *serial_rx_thread(void *arg) {
    char buf[256];
    while (g_thread_running) {
        int n = read(g_serial_fd, buf, sizeof(buf));
        if (n > 0 && g_rx_cb) {
            g_rx_cb(buf, n);
        }
        // 可以适当sleep，防止CPU占用过高
        usleep(10000);
    }
    return NULL;
}

// 启动串口接收线程
int serial_start_rx(int fd) {
    g_serial_fd = fd;
    g_thread_running = 1;
    return pthread_create(&g_rx_thread, NULL, serial_rx_thread, NULL);
}

// 停止串口接收线程
void serial_stop_rx() {
    g_thread_running = 0;
    pthread_join(g_rx_thread, NULL);
}

volatile int goto_music = 0;
volatile int goto_music_pause = 0;
volatile int goto_music_play = 0;
volatile int goto_music_next = 0;
volatile int goto_music_prev = 0;
volatile int return_time_flag = 0;
volatile int return_date_flag = 0;
volatile int return_week_flag = 0;
volatile int return_weather_flag = 0;
volatile int goto_backup_flag = 0;
volatile int goto_home_flag = 0;


void my_rx_callback(const char *data, int len) {
    // printf("收到数据: %.*s\n", len, data);
    if (len > 0 && ((data[0] == 'b' && data[1] == '4') || (data[0] == 'b' && data[1] == '6'))) {
        goto_music = 1; // 只设置标志位
    }
    if (len > 0 && ((data[0] == 'b' && data[1] == '7') || (data[0] == 'b' && data[1] == '8'))) {
        goto_music_pause = 1; // 只设置标志位
    }    
    if (len > 0 && ((data[0] == 'b' && data[1] == '9') || (data[0] == 'b' && data[1] == 'a'))) {
        goto_music_play = 1; // 只设置标志位
    }     
    if (len > 0 && (data[0] == 'b' && data[1] == 'b')) {
        goto_music_next = 1; // 只设置标志位
    }  
    if (len > 0 && (data[0] == 'b' && data[1] == 'c')) {
        goto_music_prev = 1; // 只设置标志位
    } 

    if (len > 0 && (data[0] == 'b' && data[1] == '0')) {
        return_time_flag = 1; // 只设置标志位
    }  
    if (len > 0 && (data[0] == 'b' && data[1] == '2')) {
        return_date_flag = 1; // 只设置标志位
    }  
    if (len > 0 && (data[0] == 'b' && data[1] == '1')) {
        return_week_flag = 1; // 只设置标志位
    } 
    if (len > 0 && (data[0] == 'b' && data[1] == '3')) {
        return_weather_flag = 1; // 只设置标志位
    } 
    if (len > 0 && (data[0] == 'c' && data[1] == '0')) {
        goto_backup_flag = 1; // 只设置标志位
    } 
    if (len > 0 && (data[0] == 'c' && data[1] == '1')) {
        goto_home_flag = 1; // 只设置标志位
    } 

}
