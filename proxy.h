#ifndef PROXY_H_INCLUDED
#define PROXY_H_INCLUDED

#include <iostream>
#include <WinSock2.h>
#include <windows.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#define BUFSIZE 8192
#define HLEN 128

void hide()
{
    CONSOLE_CURSOR_INFO cci;
    GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
    cci.bVisible = 0;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
}

class Proxy{
    SOCKET sfd;
    int local_port;
    char HOST[16];
public:
    Proxy();
    void setup(char* ht);
    int start(int lport);
    SOCKET* accept_cfd();
    void get_host(char* buff,char *host, int* port);  // 获取远端地址 host:port
    void del_line(char* str, char* key);
    char* rebuild(char* buff, int* length);
    SOCKET* connect_host(char* host,int port);
};

Proxy::Proxy()
{
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 1), &wsaData);
    memset(HOST, 0, 16);
}
void Proxy::setup(char* ht)
{
    int hol = strlen(ht);
    if(hol > 1)
    {
        strncpy(HOST, ht, hol);
    }else{
        strcpy(HOST, "Host:");
    }
}
int Proxy::start(int lport)
{
    local_port = lport;
    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return -1;
    }
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(local_port);

    if((bind(sfd, (struct sockaddr*)&local, sizeof(local))) != 0){
        return -2;
    }
    if((listen(sfd, 20)) < 0){
        return -3;
    }
    return 0;
}

SOCKET* Proxy::accept_cfd()
{
    SOCKET* cfd = new SOCKET;
    struct sockaddr_in cli_addr;
    int len = sizeof(cli_addr);

    *cfd = accept(sfd,(struct sockaddr*)&cli_addr,&len);
    if(*cfd <= 0){
        return NULL;
    }
    return cfd;
}

void Proxy::get_host(char* buff,char *host, int* port)
{
    // Host: www.baidu.com:443
    int hl = strlen(HOST);
	char* p = strstr(buff,HOST);
	if(p)
    {
        char* p1 = strstr(p, "\r\n");
        char* p2 = strchr(p + hl, ':');
        memset(host, 0, HLEN);
        if(p2 && p2 < p1)
        {
            int len = (int)(p2 - p - 1 - hl);
            memcpy(host, p + 1 + hl, len);
            *port = atoi(p2 + 1);
        }else{
            int len = (int)(p1 - p - 1 - hl);
            memcpy(host, p + 1 + hl, len);
            *port = 80;
        }
    }
}
void Proxy::del_line(char* str, char* key)
{
    int header_len = strlen(str);
    char *next_line, *p;

    for (p = strstr(str, key); p != NULL; p = strstr(p-1, key))
    {
        next_line = strchr(p, '\n');
        if (next_line++)
        {
            memcpy(p, next_line, header_len - (next_line - str) + 1); //+1是为了复制最后的\0
            header_len -= next_line - p;
        }
    }
}
// url 转换成 uri 形式 一般而言长度多会减少 理论不会有内存溢出
char* Proxy::rebuild(char *buff, int *header_len)
{
    char del[] = "Host:";
    char *p, *p0, *pr, *p1, *p2, *p3, *p4, *p5, *phost;
    int hl, len;
    hl = strlen(HOST);

    p = strstr(buff, "http://");
    p0 = strstr(p, "HTTP");
    p1 = strchr(p + 7, '/');
    if(p)
    {
        if(p1 < p0)
        {
            sprintf(p, "%s", p1);  // url -> uri
        }else{
            pr = strchr(p, ' ');  // http://aaa.com HTTP
            sprintf(p, "/%s", pr);
        }
    }
    p2 = strstr(buff, HOST) + hl;
    if(!p2) { goto end0; }
    p3 = strstr(p2, "\r\n");
    len = (int)(p3 - p2 - 1);
    if((phost = new char[len + 1]) == NULL)
    {
        goto end1;
    }
    memcpy(phost, p2 + 1, len);  // 获取host
    *(phost + len) = '\0';

    del_line(buff, del);   // 删除指定一行
    del_line(buff, HOST);

    p4 = strstr(buff, "HTTP/1") + 10;
    len = strlen(p4);
    if((p5 = new char[len + 1]) == NULL)
    {
        goto end1;
    }
    memcpy(p5, p4, len);
    *(p5 + len) = '\0';
    sprintf(p4, "Host: %s\r\n%s",phost,p5);   // 在首行后面添加一行Host：

    delete p5;
end1:
    delete phost;
end0:
    *header_len = strlen(buff);
    return buff;
}

SOCKET* Proxy::connect_host(char* host,int port)
{
    SOCKET* rfd = new SOCKET;
    struct sockaddr_in remote_addr;
    struct hostent *server;

    if ((*rfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return NULL;
    }

    if((server = gethostbyname(host)) == NULL)
    {
        return NULL;
    }
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    memcpy(&remote_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    remote_addr.sin_port = htons(port);

    if (connect(*rfd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) < 0){
        delete rfd;
        return NULL;
    }
    return rfd;
}

#endif // PROXY_H_INCLUDED
