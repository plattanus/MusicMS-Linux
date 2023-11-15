#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <termios.h>
#include <sqlite3.h>
#define MUSIC_NAME_LENGTH 100
#define MAXN 10000
int sockfd = 0;
char userIdentity = '0';
char music_name[MUSIC_NAME_LENGTH][MUSIC_NAME_LENGTH];
char str[MUSIC_NAME_LENGTH];
// 保存用户名和密码
char username[50], password[15];
// 音乐列表
void mp3list(void);
// 读取操作选项
int get_option(void);
// 项目作者信息
void manual(void);
// 上一首歌曲
void last_song(void);
// 下一首歌曲
void next_song(void);
// 播放音乐
void play_music(void);
// 可播放歌曲的个数
int indexmusic=0;
// 正在播放歌曲
int nowmusic=0;
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
char* get_str(char* str, size_t len)
{
    if(NULL == str)
    {
        puts("空指针异常");
        return NULL;
    }
    char *in = fgets(str, len, stdin);
    if(NULL == in)
        return str;
    size_t cnt = strlen(str);
    if('\n' == str[cnt-1])
    {
        str[cnt-1] = '\0';
    }
    stdin->_IO_read_ptr = stdin->_IO_read_end;//清理输入缓冲区
    return str;
}
char get_cmd(char start,char end)
{
    stdin->_IO_read_ptr = stdin->_IO_read_end;//清理输入缓冲区
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
// 菜单
void menu(void);
// 上传
void upload(void);
// 下载
void download(void);
// 删除服务器文件
void s_delete(void);
// 删除客户端文件
void c_delete(void);
// 显示服务器目录和文件
void s_list(void);
// 显示客户端目录和文件
void c_list(void);
// 选择音乐播放
void c_music_play(void);
// 退出程序
void quit(void);
// 登录系统
void login(void);
// 主函数
int main()
{
    printf("服务器创建socket...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > sockfd)
    {
        perror("socket");
        return -1;
    }
    printf("准备地址...\n");
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    // 端口自己修改
    addr.sin_port = htons(60000);
    // IP自行修改
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t len = sizeof(addr);
    printf("绑定连接服务器...\n");
    if (connect(sockfd, (struct sockaddr *)&addr, len))
    {
        perror("connect");
        return -1;
    }
    chdir("clientfile");
    login();
    menu(); // 加载菜单
    close(sockfd);
    return 0;
}
void login()
{
    char str[MAXN];
    //以只读的方式打开一个文件
    int fd = open( "user.log", O_RDONLY );
    if (-1==fd) //打开出错
    {
        printf("用户信息文件打开失败\n");
        exit(1);
    }
    printf("===========================================================\n");
    printf("                           进入系统                        \n");
    //读取刚才写入的十条信息，并在频幕显示
    read(fd, str, sizeof(str));
    printf("===============账号：");
    scanf("%s",username);
    printf("===============密码：");
    scanf("%s",password);
    printf("===========================================================\n");
    int oklogin=0;
    char delim[] = " ";
    char *pstr = NULL;
    int nowcnt=0;
    char _username[50], _password[15];
    for(pstr = strtok(str, delim); pstr != NULL; pstr = strtok(NULL, delim))
    {
        if(pstr[1]=='\n')
        {
            userIdentity=pstr[0];
            if(strcmp(username,_username)==0&&strcmp(password,_password)==0)
            {
                oklogin=1;
                break;
            }
            strcpy(_username,"");
            strcpy(_password,"");
            nowcnt=1;
            strcat(_username, pstr+2);
            continue;
        }
        nowcnt++;
        if(nowcnt==1)
        {
            strcat(_username, pstr);
        }
        if(nowcnt==2)
        {
            strcat(_password, pstr);
        }
    }
    if(oklogin==0)
    {
        printf("*************************登录失败**************************\n");
        exit(0);
    }
}
// 加载菜单
void menu(void)
{
    printf("*************************登录成功**************************\n");
    if(userIdentity=='1')
    {
        printf("管理员进入系统\n");
        while(1)
        {
            printf("===========================================================\n");
            printf("                           系统使用                        \n");
            printf("         1、上传音乐                   2、下载音乐          \n");
            printf("         3、删除服务器端音乐           4、删除客户器端音乐  \n");
            printf("         5、服务器端音乐               6、客户端音乐        \n");
            printf("         7、音乐播放                   0、退出              \n");
            printf("===========================================================\n");
            switch (get_cmd('0', '7'))
            {
            case '1':
                upload();
                break;
            case '2':
                download();
                break;
            case '3':
                s_delete();
                break;
            case '4':
                c_delete();
                break;
            case '5':
                s_list();
                break;
            case '6':
                c_list();
                break;
            case '7':
                c_music_play();
                break;
            case '0':
                quit();
                return;
            }
        }
    }
    else
    {
        printf("普通客户进入系统\n");
        while(1)
        {
            printf("===========================================================\n");
            printf("                           系统使用                        \n");
            printf("         1、下载音乐                   2、删除客户器端音乐  \n");
            printf("         3、服务器端音乐               4、客户端音乐        \n");
            printf("         5、音乐播放                   0、退出              \n");
            printf("===========================================================\n");
            switch (get_cmd('0', '5'))
            {
            case '1':
                download();
                break;
            case '2':
                c_delete();
                break;
            case '3':
                s_list();
                break;
            case '4':
                c_list();
                break;
            case '5':
                c_music_play();
                break;
            case '0':
                quit();
                return;
            }
        }
    }
}
// 上传
void upload(void)
{
    char up[20] = "上传文件";
    write(sockfd, up, strlen(up) + 1);
    // 打印当前目录文件
    c_list();
    int r_size = 0;
    int w_size = 0;
    char buf[1024] = {};
    char buf2[20] = {};
    r_size = read(sockfd, buf, sizeof(buf));
    if(strncmp(buf, "success", 10) != 0)
    {
        printf("收到服务端异常数据\n");
        getch();
        return;
    }
    char pathname[100] = {};
    char *filename = malloc(50);
    memset(filename, 0, 50);
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select song from musicusers where username=\'";
    strcat(sql,username);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    while(1)
    {
        printf("请输入歌曲名：");
        get_str(pathname, 100);
        printf("%s\n",pathname);
        int ok=0;
        for(i=1; i<row+1; i++)
        {
            for(j=0; j<col; j++)
            {
                //该用户数据库存在该音乐文件
                if(strcmp(pathname,Result[j+i*col])==0)
                {
                    ok=1;
                    break;
                }
            }
            if(ok==1)
                break;
        }
        if(ok==1)
            break;
        printf("该音乐文件不存在，请重新输入：");
    }
    int fd = open(pathname, O_RDONLY);
    if (fd == -1)
    {
        printf("歌曲文件不存在\n");
        write(sockfd, "error", 6);
        getch();
    }
    else
    {
        write(sockfd, "success", 8);
        usleep(100000);
        if (strrchr(pathname, '/') == NULL)
        {
            strncpy(filename, pathname, 50);
        }
        else
        {
            filename = strrchr(pathname, '/');
            filename += 1;
        }
        printf("发送歌曲名：%s 至服务端\n", filename);
        write(sockfd, filename, strlen(filename) + 1);
        usleep(100000);
        memset(buf, 0, sizeof(buf));
        r_size = read(sockfd, buf, sizeof(buf));
        if(strncmp(buf, "success", 10) != 0)
        {
            printf("收到服务端异常数据\n");
            getch();
            return;
        }
        else
        {
            printf("收到服务端返回success\n");
        }
        sleep(1);
        //设置文件读写位置为文件尾部
        lseek(fd, 0, SEEK_END);
        // 获取文件字节数（尾部位置）
        off_t end_pos = lseek(fd, 0, SEEK_CUR);
        //设置文件读写位置为文件头部
        lseek(fd, 0, SEEK_SET);
        do
        {
            r_size = read(fd, buf, 1024);
            printf("[读取歌曲文件字节数：%d ", r_size);
            w_size = write(sockfd, buf, r_size);
            printf("发送字节数：%d ", w_size);
            read(sockfd, buf2, sizeof(buf2));
            if(strncmp(buf2, "success", 10) == 0)
            {
                printf("成功收到服务端返回的success]\n");
            }
            usleep(10000);
            off_t cur_pos = lseek(fd, 0, SEEK_CUR);
            if(cur_pos == end_pos && w_size == 1024)
            {
                char end[1] = "\0";
                printf("[读取歌曲文件字节数：1 ");
                w_size = write(sockfd, end, sizeof(end));
                printf("发送字节数：%d ", w_size);
                read(sockfd, buf2, sizeof(buf2));
                if(strncmp(buf2, "success", 10) == 0)
                {
                    printf("成功收到服务端返回的success]\n");
                }
                else
                {
                    printf("收到服务端返回的异常数据：%s]\n", buf2);
                }
                break;
            }
        }
        while (r_size == 1024);
        close(fd);
        char result[20] = {};
        read(sockfd, result, sizeof(result));
        if(strncmp(buf2, "success", 10) == 0)
        {
            printf("成功收到服务端返回值：%s,服务器接收歌曲文件成功\n", result);
        }
        else if(strncmp(buf2, "error", 10) == 0)
        {
            printf("成功收到服务端返回值：%s,服务器接收歌曲文件异常\n", result);
        }
        else
        {
            printf("收到服务端返回值：%s,数据异常\n", result);
        }
        getch();
    }
}
// 下载
void download(void)
{
    int r_size = 0;
    int w_size = 0;
    char buf[1024] = {};
    char filename[50] = {};
    char tfilename[50]= {};
    char fileauthor[50]= {};
    char list[1024] = {};
    char down[20] = "下载文件";
    write(sockfd, down, strlen(down) + 1);
    r_size = read(sockfd, buf, sizeof(buf));
    if(strncmp(buf, "success", 10) != 0)
    {
        printf("收到服务端异常数据\n");
        getch();
        return;
    }
    else
    {
        printf("服务端成功接收命令\n");
    }
    read(sockfd, list, sizeof(list));
    printf("服务端目录列表：\n%s\n", list);
    usleep(1000);
    while(1)
    {
        printf("请输入要下载的歌曲名：");
        get_str(filename, 50);
        int i;
        int ok=0;
        int i_index=0;
        int j_index=0;
        for(i=0; i<strlen(list); i++)
        {
            if(list[i]=='\n')
            {
                i_index=0;
                continue;
            }
            if(list[i]==' ')
            {
                tfilename[i_index]='\0';
                if(strcmp(filename,tfilename)==0)
                {
                    ok=1;
                    i++;
                    while(list[i]!='\n')
                    {
                        fileauthor[j_index++]=list[i];
                        i++;
                    }
                    fileauthor[j_index]='\0';
                    break;
                }
                continue;
            }
            tfilename[i_index++]=list[i];
        }
        if(ok==1)
            break;
        printf("该音乐文件不存在，请重新输入：");
    }
    write(sockfd, filename, strlen(filename) + 1);
    char result[20] = {};
    read(sockfd, result, sizeof(result));
    if(strncmp(result, "success", 8) == 0)
    {
        printf("收到服务端发送的数据：%s 歌曲文件准备下载\n", result);
    }
    else if(strncmp(result, "error", 8) == 0)
    {
        printf("收到服务端发送的数据：%s 歌曲文件不存在\n", result);
        getch();
        return;
    }
    else
    {
        printf("收到服务端发送的数据：%s 数据异常,下载终止\n", result);
        getch();
        return;
    }
    int fd = open(filename, O_CREAT | O_RDWR, 0777);
    do
    {
        usleep(500);
        memset(buf, 0, sizeof(buf));
        r_size = read(sockfd, buf, sizeof(buf));
        printf("[收到字节数：%d ", r_size);
        w_size = write(fd, buf, r_size);
        printf("写入歌曲文件字节数：%d ", w_size);
        w_size = write(sockfd, "success", 8);
        printf("发送success给服务端]\n");
    }
    while (r_size == 1024);
    usleep(1000000);
    printf("歌曲文件：%s 下载完毕\n", filename);
    //将新下载的文件存入数据库
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char sql[100]="insert into musicusers (username, song, author) values (\'";
    strcat(sql,username);
    strcat(sql,"\', \'");
    strcat(sql,filename);
    strcat(sql,"\', \'");
    strcat(sql,fileauthor);
    strcat(sql,"\')");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    close(fd);
    getch();
}
// 删除服务器端文件
void s_delete(void)
{
    char buf[1024] = {};
    char filename[50] = {};
    char tfilename[50] = {};
    char list[1024] = {};
    char delete[20] = "删除文件";
    write(sockfd, delete, strlen(delete) + 1);
    read(sockfd, buf, sizeof(buf));
    if(strncmp(buf, "success", 10) != 0)
    {
        printf("收到服务端异常数据\n");
        getch();
        return;
    }
    else
    {
        printf("服务端成功接收命令\n");
    }
    read(sockfd, list, sizeof(list));
    printf("服务端目录列表：\n%s\n", list);
    usleep(1000);
    while(1)
    {
        printf("请输入要删除的歌曲名：");
        get_str(filename, 50);
        int i;
        int ok=0;
        int i_index=0;
        for(i=0; i<strlen(list); i++)
        {
            if(list[i]=='\n')
            {
                tfilename[i_index]='\0';
                if(strcmp(filename,tfilename)==0)
                {
                    ok=1;
                    break;
                }
                i_index=0;
                continue;
            }
            tfilename[i_index++]=list[i];
        }
        if(ok==1)
            break;
        printf("该音乐文件不存在，请重新输入：");
    }
    write(sockfd, filename, strlen(filename) + 1);
    char result[20] = {};
    read(sockfd, result, sizeof(result));
    if(strncmp(result, "success", 8) == 0)
    {
        printf("收到服务端发送的数据：%s 歌曲文件准备删除\n", result);
    }
    else if(strncmp(result, "error", 8) == 0)
    {
        printf("收到服务端发送的数据：%s 歌曲文件不存在\n", result);
        getch();
        return;
    }
    else
    {
        printf("收到服务端发送的数据：%s 数据异常,删除终止\n", result);
        getch();
        return;
    }
    printf("删除成功\n");
    getch();
}
// 删除客户端文件
void c_delete(void)
{
    char filename[50] = {};
    printf("客户端目录列表：\n");
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musicusers where username=\'";
    strcat(sql,username);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    for(i=1; i<row+1; i++)
    {
        for(j=1; j<col; j++)
        {
            printf("%s ",Result[j+i*col]);
        }
        printf("\n");
    }
    usleep(1000);
    while(1)
    {
        printf("请输入要删除的歌曲名：");
        get_str(filename, 50);
        int ok=0;
        for(i=1; i<row+1; i++)
        {
            if(strcmp(filename,Result[1+i*col])==0)
            {
                ok=1;
                break;
            }
        }
        if(ok==1)
            break;
        printf("该音乐文件不存在，请重新输入：");
    }
    strcpy(sql,"delete from musicusers where song=\'");
    strcat(sql,filename);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    printf("删除成功\n");
    getch();
}
// 服务端文件列表
void s_list(void)
{
    char see[20] = "查看服务";
    write(sockfd, see, strlen(see) + 1);
    char list[1024] = {};
    read(sockfd, list, sizeof(list));
    printf("服务端目录列表：\n%s\n", list);
    getch();
}
// 客户端文件列表
void c_list(void)
{
    printf("当前目录列表：\n");
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musicusers where username=\'";
    strcat(sql,username);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    for(i=0; i<row+1; i++)
    {
        for(j=0; j<col; j++)
        {
            printf("%s ",Result[j+i*col]);
        }
        printf("\n");
    }
    getch();
}
// 选择音乐播放
void c_music_play(void)
{
    int option;
    // 正在播放歌曲
    nowmusic=0;
    printf("===========================================================\n");
    printf("                           播放音乐                        \n");
    printf("         1、使用手册                   2、查看歌单          \n");
    printf("         3、暂停播放                   4、继续播放          \n");
    printf("         5、上一首歌                   6、下一首歌          \n");
    printf("         7、播放歌曲                   0、退出              \n");
    printf("===========================================================\n");
    // 先给已存在的音乐标号
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musicusers where username=\'";
    strcat(sql,username);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    indexmusic=0;
    for(i=1; i<row+1; i++)
    {
        for(j=1; j<col; j++)
        {
            if(j==1)
            {
                strcpy(music_name[indexmusic], Result[j+i*col]);
                indexmusic++;
            }
        }
    }
    while(1)
    {
        option=get_option();
        switch (option)
        {
        case 1:
            manual();
            break;
        case 2:
            mp3list();
            break;
        case 3:
            system("killall -STOP madplay &");
            printf("已暂定\n");
            break;
        case 4:
            system("killall -CONT madplay &");
            printf("已继续\n");
            break;
        case 5:
            last_song();
            break;
        case 6:
            next_song();
            break;
        case 7:
            play_music();
            break;
        case 0:
            printf("退出播放音乐应用\n");
//            exit_music();
            return;
        default:
            printf("输入错误，请重新输入\n");
            break;
        }
    }
}
// 退出程序
void quit(void)
{
    printf("告知服务端，退出程序");
    char quit[20] = "退出程序";
    write(sockfd, quit, strlen(quit) + 1);
    usleep(10000);
    printf("退出应用程序\n");
}
//用户端自己的音乐操作===============================================
// 歌单列表
void mp3list(void)
{
    int row,col;
    sqlite3 *db=NULL;
    char *Errormsg;
    int rc;
    rc =sqlite3_open("musicuser.db",&db);
    if(rc)
    {
        printf("数据库\'%s\'打开失败：\n",sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    char **Result;
    char sql[100]="select * from musicusers where username=\'";
    strcat(sql,username);
    strcat(sql,"\'");
    sqlite3_exec(db,sql,0,0,&Errormsg);
    sqlite3_get_table(db,sql,&Result,&row,&col,&Errormsg);
    int i,j;
    indexmusic=0;
    int fileflag=0;
    for(i=1; i<row+1; i++)
    {
        for(j=1; j<col; j++)
        {
            if(j==1)
            {
                fileflag=1;
                strcpy(music_name[indexmusic], Result[j+i*col]);
                printf("%d：",indexmusic);
                indexmusic++;
            }
            printf("%s ",Result[j+i*col]);
        }
        printf("\n");
    }
    if(fileflag==0)
    {
        printf("当前路径下面没有歌曲(.mp3格式)文件\n");
        printf("请先下载歌曲文件\n");
    }
}
// 读取音乐播放器操作选项
int get_option(void)
{
    int option;
    printf("请输入操作(1使用手册)：");
    scanf("%d", &option);
    getchar();
    return option;
}
// 项目信息
void manual(void)
{
    char n;
    printf("创作者：李腾腾\n");
    printf("创作日期：2021/6/19\n");
    printf("===========================================================\n");
    printf("                           播放音乐                         \n");
    printf("         1、使用手册                   2、查看歌单            \n");
    printf("         3、暂停播放                   4、继续播放            \n");
    printf("         5、上一首歌                   6、下一首歌            \n");
    printf("         7、播放歌曲                   0、退出                \n");
    printf("===========================================================\n");
    printf("根据菜单前面的标号既可操作相对应的功能\n");
    printf("进入系统后，如果存在歌曲定位第一首歌曲\n");
    printf("歌曲的暂停、继续等有歌曲播放过才能生效\n");
    printf("***********************************************************\n");
    printf("输入q/Q退出使用手册：");
    while(1)
    {
        scanf("%c", &n);
        if(n=='q'||n=='Q')
        {
            break;
        }
        else
        {
            printf("输入错误请重新输入：");
        }
    }
}
// 上一首歌曲
void next_song(void)
{
    char next_buf[MUSIC_NAME_LENGTH]= {0};
    memset(str,0,sizeof(str));
    system("killall -9 madplay");
    nowmusic++;
    if(nowmusic==indexmusic)
    {
        nowmusic=0;
    }
    printf("当前播放歌曲：%s\n",music_name[nowmusic]);
    printf("上一首歌曲已经播放\n");
    sprintf(next_buf,"madplay -v %s &",music_name[nowmusic]);
    system(next_buf);
}
// 下一首歌曲
void last_song(void)
{
    char last_buf[MUSIC_NAME_LENGTH]= {0};
    memset(str,0,sizeof(str));
    system("killall -9 madplay");
    nowmusic--;
    if(nowmusic==-1)
    {
        nowmusic=indexmusic-1;
    }
    printf("当前播放歌曲：%s\n",music_name[nowmusic]);
    printf("下一首歌曲已经播放\n");
    sprintf(last_buf,"madplay -v %s &",music_name[nowmusic]);
    system(last_buf);
}
// 播放音乐
void play_music(void)
{
    int i;
    char buf[MUSIC_NAME_LENGTH];
    char *music_path;
    char path_str[MUSIC_NAME_LENGTH]= {0};
    printf("请输入你需要播放的歌曲的标号：");
    scanf("%d", &nowmusic);
    getchar();
    for(i=0; i<indexmusic; i++)
    {
        if(nowmusic==i)
        {
            music_path=strcpy(path_str,music_name[i]);
            printf("当前播放歌曲：%s\n",music_name[i]);
            sprintf(buf,"madplay -v %s &",music_path);
            system(buf);
        }
    }
}





