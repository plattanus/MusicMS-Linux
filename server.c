#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <termios.h>
#include <sqlite3.h>
// 开始运行
void *start_run(void *arg);
// 客户端上传
void c_up(int *clifd);
// 客户端下载
void c_down(int *clifd);
// 客户端删除
void c_delete(int *clifd);
// 返回文件列表
void c_list(int *clifd);
//　修改终端的控制方式，1取消回显、确认　２获取数据　3还原
int getch(void)
{
    // 记录终端的配置信息
    struct termios old;
    // 获取终端的配置信息
    tcgetattr(STDIN_FILENO,&old);
    // 设置新的终端配置
    struct termios new1 = old;
    // 取消确认、回显
    new1.c_lflag &= ~(ICANON|ECHO);
    // 设置终端配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&new1);
    // 在新模式下获取数据
    int key_val = 0;
    do
    {
        key_val += getchar();
    }
    while(stdin->_IO_read_end - stdin->_IO_read_ptr);
    // 还原配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&old);
    return key_val;
}
char get_cmd(char start,char end)
{
    //清理输入缓冲区
    stdin->_IO_read_ptr = stdin->_IO_read_end;
    printf("请输入指令：");
    while(true)
    {
        char val = getch();
        if(val >= start && val <= end)
        {
            printf("%c\n",val);
            return val;
        }
    }
}
// 主函数
int main()
{
    printf("服务器创建socket...\n");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("准备地址...\n");
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    // 端口 IP 自行修改
    addr.sin_port = htons(60000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(addr);
    printf("绑定socket与地址...\n");
    if (bind(sockfd, (struct sockaddr *)&addr, len))
    {
        perror("bind");
        return -1;
    }
    printf("设置监听...\n");
    if (listen(sockfd, 5))
    {
        perror("listen");
        return -1;
    }
    printf("等待客户端连接...\n");
    chdir("serverfile");
    while(1)
    {
        struct sockaddr_in addrcli = {};
        int *clifd = (int*)malloc(sizeof(int));
        *clifd = accept(sockfd, (struct sockaddr *)&addrcli, &len);
        if (0 > *clifd)
        {
            perror("accept");
            continue;
        }
        pthread_t pid;
        // 创建线程函数
        // 第一个参数为指向线程标识符的指针
        // 第二个参数用来设置线程属性
        // 第三个参数是线程运行函数的地址
        // 最后一个参数是运行函数的参数
        pthread_create(&pid, NULL, start_run, (void *)clifd);
    }
    return 0;
}
// 开始运行
void *start_run(void *arg)
{
    char cmd[20] = {0};
    int *clifd = (int *)arg;
    char up[20] = "上传文件";
    char down[20] = "下载文件";
    char delete[20] = "删除文件";
    char see[20] = "查看服务";
    char quit[20] = "退出程序";
    int c_size = 0;
    while(1)
    {
        c_size = read(*clifd, cmd, sizeof(cmd));
        if(-1 == c_size)
        {
            printf("read函数出错\n");
        }
        if (strcmp(up, cmd) == 0)
        {
            printf("收到客户端的上传指令\n");
            c_up(clifd);
            memset(cmd, 0, 20);
        }
        else if (strcmp(down, cmd) == 0)
        {
            printf("收到客户端的下载指令\n");
            c_down(clifd);
            memset(cmd, 0, 20);
        }
        else if (strcmp(delete, cmd) == 0)
        {
            printf("收到客户端的删除指令\n");
            c_delete(clifd);
            memset(cmd, 0, 20);
        }
        else if (strcmp(see, cmd) == 0)
        {
            printf("收到客户端的目录指令\n");
            c_list(clifd);
            memset(cmd, 0, 20);
        }
        else if (strcmp(quit, cmd) == 0)
        {
            printf("收到服务端的退出指令\n");
            pthread_exit(0);
            return (void *)NULL;
        }
    }
}
// 上传
void c_up(int *clifd)
{
    int flag = 0;
    int r_size = 0;
    int w_size = 0;
    char buf[1024] = {};
    w_size = write(*clifd, "success", 8);
    read(*clifd, buf, 10);
    if(strncmp(buf, "error", 10) == 0)
    {
        printf("收到客户端返回error,接收终止\n");
        return;
    }
    else if(strncmp(buf, "success", 10) == 0)
    {
        printf("收到客户端返回success,继续接收\n");
    }
    else
    {
        printf("收到客户端异常数据：%s,接收终止\n", buf);
        return;
    }
    char filename[50] = {};
    memset(filename, 0, sizeof(filename));
    int f_size = read(*clifd, filename, sizeof(filename));
    if(-1 == f_size)
    {
        printf("read函数出错\n");
    }
    printf("收到歌曲名：%s\n", filename);
    usleep(100000);
    w_size = write(*clifd, "success", 8);
    printf("发送success给客户端\n");
    int fd = open(filename, O_CREAT | O_RDWR, 0777);
    do
    {
        memset(buf, 0, sizeof(buf));
        r_size = read(*clifd, buf, sizeof(buf));
        printf("[收到字节数：%d ", r_size);
        w_size = write(fd, buf, r_size);
        printf("写入歌曲文件字节数：%d ", w_size);
        usleep(10000);
        w_size = write(*clifd, "success", 8);
        printf("发送success给客户端]\n");
        flag++;
    }
    while (r_size == 1024);
    sleep(1);
    if (flag > 0)
    {
        char result[20] = "success";
        printf("歌曲文件传输完毕，返回客户端success\n");
        write(*clifd, result, strlen(result) + 1);
        //传输成功后，文件信息加入数据库
        sqlite3 *db=NULL;
        char *Errormsg;
        int rc;
        rc =sqlite3_open("music.db",&db);
        if(rc)
        {
            printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        char sql[100]="insert into musics (song, author) values(\'";
        strcat(sql,filename);
        strcat(sql,"\', \'");
        strcat(sql,"unknow\')");
        sqlite3_exec(db,sql,0,0,&Errormsg);
    }
    else
    {
        char result[20] = "error";
        printf("歌曲文件传输失败，返回客户端error\n");
        write(*clifd, result, strlen(result) + 1);
    }
    close(fd);
}
// 下载
void c_down(int *clifd)
{
    char list[1024] = {};
    int r_size = 0;
    int w_size = 0;
    char buf[1024] = {};
    char buf2[20] = {};
    char filename[50] = {};
    char filename2[51] = {};
    usleep(10000);
    w_size = write(*clifd, "success", 8);
    usleep(10000);
    // 获取目录下所有文件名
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("music.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musics";
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    for(i=1; i<row+1; i++)
    {
        for(j=0; j<col; j++)
        {
            if(j!=0)
                strcat(list, " ");
            strcat(list, Result[j+i*col]);
        }
        strcat(list, "\n");
    }
    printf("当前目录列表：\n%s\n", list);
    printf("strlen(list)：%d\n", (int)strlen(list));
    int l_size = write(*clifd, list, strlen(list)+1);
    if(-1 == l_size)
    {
        printf("read函数出错\n");
    }
    printf("发送当前下载目录列表给客户端\n");
    printf("等待接收歌曲名...\n");
    int f_size = read(*clifd, filename, sizeof(filename));
    if(-1 == f_size)
    {
        printf("read函数出错\n");
    }
    strncpy(filename2, filename, 50);
    printf("filename：%s\n", filename2);
    if (strstr(list, filename2) == NULL || strncmp(filename2, " ", 1) == 0 || strncmp(filename2, "  ", 2) == 0)
    {
        char result[6] = "error";
        printf("歌曲文件：%s 不存在,下载终止\n", filename);
        write(*clifd, result, strlen(result));
        return;
    }
    else
    {
        char result[8] = "success";
        printf("歌曲文件：%s 存在,开始传输歌曲文件内容\n", filename);
        write(*clifd, result, strlen(result));
        int fd = open(filename, O_RDONLY);
        //设置文件读写位置为文件尾部
        lseek(fd, 0, SEEK_END);
        // 获取文件字节数（尾部位置）
        off_t end_pos = lseek(fd, 0, SEEK_CUR);
        //设置文件读写位置为文件头部
        lseek(fd, 0, SEEK_SET);
        usleep(1000000);
        do
        {
            memset(buf, 0, sizeof(buf));
            r_size = read(fd, buf, sizeof(buf));
            printf("[读取歌曲文件字节数：%d ", r_size);
            w_size = write(*clifd, buf, r_size);
            printf("发送字节数：%d ", w_size);
            read(*clifd, result, sizeof(result));
            if(strncmp(result, "success", 10) == 0)
            {
                printf("成功收到客户端端返回的success]\n");
            }
            usleep(10000);
            off_t cur_pos = lseek(fd, 0, SEEK_CUR);
            if(cur_pos == end_pos && w_size == 1024)
            {
                char end[1] = "\0";
                printf("[读取歌曲文件字节数：1 ");
                w_size = write(*clifd, end, sizeof(end));
                printf("发送字节数：%d ", w_size);
                read(*clifd, buf2, sizeof(buf2));
                if(strncmp(buf2, "success", 10) == 0)
                {
                    printf("成功收到客户端端返回的success]\n");
                }
                else
                {
                    printf("收到客户端返回的异常数据：%s]\n", buf2);
                }
                break;
            }
        }
        while (r_size == 1024);
        usleep(1000000);
        printf("歌曲文件：%s 发送完毕\n", filename);
        close(fd);
    }
}
// 删除
void c_delete(int *clifd)
{
    char list[1024] = {};
    char filename[50] = {};
    char filename2[51] = {};
    usleep(10000);
    write(*clifd, "success", 8);
    usleep(10000);
    // 获取目录下所有文件名
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("music.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select song from musics";
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    for(i=1; i<row+1; i++)
    {
        for(j=0; j<col; j++)
        {
            if(j!=0)
                strcat(list, " ");
            strcat(list, Result[j+i*col]);
        }
        strcat(list, "\n");
    }
    printf("当前目录列表：\n%s\n", list);
    printf("strlen(list)：%d\n", (int)strlen(list));
    int l_size = write(*clifd, list, strlen(list)+1);
    if(-1 == l_size)
    {
        printf("read函数出错\n");
    }
    printf("发送当前目录列表给客户端\n");
    printf("等待接收歌曲文件名...\n");
    int f_size = read(*clifd, filename, sizeof(filename));
    if(-1 == f_size)
    {
        printf("read函数出错\n");
    }
    strncpy(filename2, filename, 50);
    printf("filename：%s\n", filename2);
    if (strstr(list, filename2) == NULL || strncmp(filename2, " ", 1) == 0 || strncmp(filename2, "  ", 2) == 0)
    {
        char result[6] = "error";
        printf("歌曲文件：%s 不存在,删除终止\n", filename);
        write(*clifd, result, strlen(result));
        return;
    }
    else
    {
        char result[8] = "success";
        printf("歌曲文件：%s 存在,删除该歌曲文件\n", filename);
        write(*clifd, result, strlen(result));
        //文件存在，删除数据库信息
        sqlite3 *db=NULL;
        char *Errormsg;
        int rc;
        rc =sqlite3_open("music.db",&db);
        if(rc)
        {
            printf("cant,t open:%s\n",sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        char sql[100]="delete from musics where song=\'";
        strcat(sql,filename);
        strcat(sql,"\'");
        sqlite3_exec(db,sql,0,0,&Errormsg);
        remove(filename);
        return;
    }
}
// 文件列表
void c_list(int *clifd)
{
    char list[1024] = {};
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("music.db",&db);
    if(rc)
    {
        printf("cant,t open:%s\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musics";
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    for(i=0; i<row+1; i++)
    {
        for(j=0; j<col; j++)
        {
            strcat(list,Result[j+i*col]);
            strcat(list," ");
        }
        strcat(list,"\n");
    }
    int l_size = write(*clifd, list, strlen(list) + 1);
    if(-1 == l_size)
    {
        printf("write函数出错\n");
    }
    memset(list, 0, sizeof(list));
}



