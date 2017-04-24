#include <process.h>
#include <fstream>
#include "proxy.h"
using namespace std;

int port;
int Hide = 0;
char hdt[12];
char SSL[12] = "CONNECT";
void info()
{
    hide();
    system("mode con: cols=33 lines=20");
    system("color F9");
    char data[100],*def;
inf:
    memset(hdt, 0, 12);
    memset(SSL, 0, 12);
    ifstream infile;
    infile.open("./set.conf", ios::in);
    while(infile.getline(data,99))
    {
        char *p = strstr(data, "Port");
        if(p)
        {
            char *pp = strchr(p, '=');
            port = atoi(pp + 1);
        }
        char *p0 = strstr(data, "Head");
        if(p0)
        {
            char *ph = strchr(p0, '=');
            char *ph0 = strchr(p0, ';');

            if(strncmp(ph, "= ", 2) == 0){
                memcpy(hdt, ph + 2, (int)(ph0 - ph - 2));
            }else{
                memcpy(hdt, ph + 1, (int)(ph0 - ph - 1));
            }
        }
        char *p1 = strstr(data, "Https");
        if(p1)
        {
            char *ps = strchr(p1, '=');
            char *ps0 = strchr(p1, ';');

            if(strncmp(ps, "= ", 2) == 0){
                memcpy(SSL, ps + 2, (int)(ps0 - ps - 2));
            }else{
                memcpy(SSL, ps + 1, (int)(ps0 - ps - 1));
            }
        }
        char *p2 = strstr(data, "Hide");
        if(p2)
        {
            char *pd = strchr(p2, '=');
            Hide = atoi(pd + 1);
        }
    }
    infile.close();

    if(port < 1 || strlen(hdt) < 1)
    {
        def = "Port = 8080;\n\nHead = Host:;\n\nHttps = CONNECT;\r\nHide = 0;\n";
        ofstream outfile;
        outfile.open("./set.conf", ios::out);
        outfile << def;
        outfile.close();
        Sleep(1500);
        goto inf;
    }
    cout << "---------------------------" << endl;
    cout << "Win Proxy Free" << endl;
    cout << "Version: 0.1\n" << endl;
    cout << "Head��\"" << hdt << "\"" << endl;
    cout << "Https: \"" << SSL << "\"" << endl;
    cout << "Config Files: set.conf" << endl;
    cout << "---------------------------" << endl;
}

unsigned int __stdcall deal_data(LPVOID sock)
{
    Proxy pr;
    SOCKET *rfd, *cfd;
    char host[HLEN], *data;
    int port, data_len;
    cfd = (SOCKET*)sock;
    data = new char[BUFSIZE];

    fd_set sfd;
    timeval tv;
    pr.setup(hdt);  // �����Զ���Host�ֶ�

    if((data_len = recv(*cfd, data, BUFSIZE, 0)) <= 0)
    {
        goto endwork1;
    }
    // ��header�л�ȡhost
    pr.get_host(data, host, &port);
    if(strlen(host) == 0)
    {
        goto endwork1;
    }
    // ��������Զ��host
    try
    {
        if((rfd = pr.connect_host(host, port)) == NULL)
        {
            throw "����Զ��ʧ��: ";
        }
    }catch(const char* err)
    {
        goto endwork1;
    }
    // ����ת��header
    if(strstr(data, "CONNECT") != NULL || strstr(data, SSL) != NULL)   // �ж��Ƿ�ΪHTTPS����
    {
        int hs;
        char sed[] = "HTTP/1.1 200 Connection Established\r\n\r\n";
        if((hs = send(*cfd, sed, strlen(sed), 0)) <= 0)
        {
            goto endwork2;
        }
    }else{
        //data = pr.rebuild(data, &data_len);
        if(send(*rfd, data, data_len, 0) <= 0)
        {
            goto endwork2;
        }
    }
    // ����ת��
    tv.tv_sec = 3;  // ������ѯ3�볬ʱ
    tv.tv_usec = 0;
    while(1)
    {
        FD_ZERO(&sfd);  // ÿ��ѭ������ն���
        FD_SET(*cfd, &sfd); // ��Ӿ��������
        FD_SET(*rfd, &sfd);
        memset(data, 0, BUFSIZE);
        int se = select(0, &sfd, NULL, NULL, &tv);  // �ȴ�����׼����
        switch(se)
        {
            case -1:  // �������� ����ѭ��
                goto endwork2;
                break;
            case 0:   // ��ʱ
                break;
            default:  // ������׼����
                if(FD_ISSET(*cfd, &sfd))  // �ж��Ƿ�ͻ��������ݿɶ�
                {   // ���ͻ������ݷ��ͳ�ȥ
                    if((data_len = recv(*cfd, data, BUFSIZE, 0)) <= 0)
                    {
                        goto endwork2;
                    }
                    if(send(*rfd, data, data_len, 0)  < 0)
                    {
                        goto endwork2;
                    }
                }
                if(FD_ISSET(*rfd, &sfd))  // �ж��Ƿ�Զ�������ݿɶ�
                {   // ��Զ�����ݽ��ջ���
                    if((data_len = recv(*rfd, data, BUFSIZE, 0)) <= 0)
                    {
                        goto endwork2;
                    }
                    if(send(*cfd, data, data_len, 0)  < 0)
                    {
                        goto endwork2;
                    }
                }
                break;
        }
    }

endwork2:
    closesocket(*rfd);
    delete rfd;
endwork1:
    closesocket(*cfd);
    delete cfd;
    delete data;
    return 0;
}

int main(int argc, char *argv[])
{
    SOCKET* cfd;
    HANDLE* hd;
    Proxy py;
    info();

    if(py.start(port) < 0){
        cout << "Listen False!" << endl;
        getchar();
        return -1;
    }else{
        cout << "Listening Port��" << port << endl;
    }
    if(Hide == 1)
    {
        cout << "\n3������ش���..." << endl;
        Sleep(2000);
        HWND hwnd = FindWindow("ConsoleWindowClass", NULL);//�ҵ���ǰ���ھ��
        if(hwnd)
        {
            ShowOwnedPopups(hwnd,SW_HIDE);//��ʾ��������ָ���������е�ȫ������ʽ����
            ShowWindow(hwnd,SW_HIDE);//���ƴ��ڵĿɼ���
        }
    }
    while(1)
    {
        client:
            cfd = py.accept_cfd();
            if(cfd <= 0){
                    Sleep(500);
                goto client;
            }
        // �������̴߳���ͻ�����
        hd = new HANDLE;
        *hd = (HANDLE)_beginthreadex(NULL,0,&deal_data,cfd,0,NULL);
        CloseHandle(*hd);
        delete hd;
    }
}
