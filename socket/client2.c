#include "global.h"
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <limits.h>
#include <netdb.h>
#include <errno.h>

#define BUFF_LEN 1024 /* 預設緩衝區長度為 1024 bytes */
#define SERVER "127.0.0.1"  /* 預設伺服器位址 */
#define PORT 12345 /* 預設port*/


int stdin_ready(void);  /* 以 select()做出non-blocking的輸入（抓時間） */


int main(int argc, char *argv[]) {
    
    int client_sockfd, nread;
    int len;
    struct sockaddr_in address;
    char server[UCHAR_MAX];
    int result;
    char buf[BUFF_LEN]; 
    char tmp[BUFF_LEN];
    char msg[BUFF_LEN];
    char nick[UCHAR_MAX];
    fd_set readfds;

    DEBUG("除錯模式啟動\n");
    
    do {
        printf("請輸入暱稱：");
        scanf("%s", nick);
    } while ((char*) NULL == nick);


    /*  建立client端 socket （ipv4)*/
    if ((client_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)  {
        perror("socket() 呼叫失敗");
        exit(EXIT_FAILURE);
    }

    /*  設定client端 socket  */  
    /* 使用者提供伺服器資訊 */
    if (argc > 1) {
        /*有輸入IP位址*/
        strcpy(server, argv[1]); /*返回argv[1]值（輸入的ip)*/
        printf("正在連線到 %s:%d ...\n", server, PORT);
    } else 
        strcpy(server, SERVER);  /* 使用預設server */

    /*bzero()函數：将内存（字符串）前n个字節清空*/
    bzero(&address, sizeof (struct sockaddr_in)); 
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER);
    address.sin_port = htons(PORT);
    len = sizeof (address);

    /* 與server建連線 */
    if ((result = connect(client_sockfd, (struct sockaddr *) &address, len)) < 0) {
        perror("connect() 呼叫失敗"); 
        close(client_sockfd);
        exit(EXIT_FAILURE);

    } else 
        printf("連線中...\n");
    
    
    FD_ZERO(&readfds);  /* 將readfds清為零，使集合中不含任何fd*/
    bzero(&buf, BUFF_LEN); /*将buf/tmp/msg前n个字節清空*/
    bzero(&tmp, BUFF_LEN);
    bzero(&msg, BUFF_LEN);
    
    FD_SET(client_sockfd, &readfds); /* 將client_sockfd 加入readfds集合*/

    
    /* 傳送接收資料 */
    printf("%s 歡迎加入聊天室\n", nick);

  /* 開始接收資料 */
    for(;;) 
    {
        if (stdin_ready())
         {
            fscanf(stdin, "\n%[^\n]", buf); /*印出現在時間*/
            /*第一個指向結構 FILE 的指標，以及第二個所要輸入的格式化字串*/
            sprintf(msg, "%s: %s", nick, buf);
            /*將nick&buf格式化傳給msg*/
            if (send(client_sockfd, msg, sizeof (msg), 0)) 
            /*send()將數據由指定的socket傳給server*/
                DEBUG("訊息已送出\n");           
        }

        usleep(100); /*暫停100微秒*/
        
        ioctl(client_sockfd, FIONREAD, &nread); 
        /*ioctl是設備驅動程序中對設備的I/O通道進行管理的函數*/
        /*FIONREAD（cmd參數)就是返回緩衝區有多少字節*/
        /*得到client_sockfd有多少字節要被讀取，在將字節數放入nread*/
        /*再來就可以讀字串了*/

        /* 處理client端資料 */
        if (!nread == 0) {             
            /* 開始接收資料 */
            if (FD_ISSET(client_sockfd, &readfds))
            /* 測試client_sockfd是否在readfds中*/
             {
                if (recv(client_sockfd, buf, sizeof (buf), 0)) 
                     DEBUG("訊息已接收\n");
                usleep(100);
                printf("%s\n", buf);
            }
        }
    }
    /* 關閉 socket */    
    close(client_sockfd);
    exit(EXIT_SUCCESS);
    
    return EXIT_SUCCESS;
}


int stdin_ready() {
    fd_set fdset;
    struct timeval timeout;
    int fd;
    
    fd = fileno(stdin);
    FD_ZERO(&fdset);
    FD_SET(fd, &fdset);
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    
    usleep(5000);    /* avoid high CPU loading */
    
    return select(fd + 1, &fdset, NULL, NULL, &timeout) == 1 ? 1 : 0;
}
