#pragma GCC diagnostic error "-std=c++11"  
#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#ifdef __cplusplus
extern "C" {
#endif
#include <iconv.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifdef __cplusplus
}
#endif
using namespace std;


vector<int> g_clients;
bool exitFlag = false;
char *out = nullptr;
class CodeConverter {

private:
    iconv_t cd;
public:

    // 构造
    CodeConverter(const char *from_charset,const char *to_charset) {
        cd = iconv_open(to_charset,from_charset);
    }


    // 析构
    ~CodeConverter() {
        iconv_close(cd);
    }


    // 转换输出
    int convert(char *inbuf,int inlen,char *outbuf,int outlen) {
        char **pin = &inbuf;
        char **pout = &outbuf;

        memset(outbuf,0,outlen);
        return iconv(cd,pin,(size_t *)&inlen,pout,(size_t *)&outlen);
    }
};
void sendMsgToclient(int _csock) {
    char sendBuf[1024];
    while(!exitFlag && find(g_clients.begin(), g_clients.end(), _csock) != g_clients.end()) {
        memset(&sendBuf, 0, sizeof(sendBuf));
        cin.getline(sendBuf, sizeof(sendBuf));
        send(_csock, (const char*)&sendBuf, sizeof(sendBuf), 0);
    }
}

bool processMsg(int _csock)
{
    char readBuf[1024];
    memset(readBuf, 0, sizeof(readBuf));
    int len = recv(_csock, readBuf, sizeof(readBuf), 0);
    if (len <= 0)
    {
        cout << "读取失败" << endl;
        return false;
    }
    else
    {
        char *out = (char*)malloc(4096);
        CodeConverter cc2 = CodeConverter("gb2312","utf-8");
        cc2.convert(readBuf, strlen(readBuf), out, 4096);
        cout <<"客户端："<< out<< endl;
        free(out);
        return true;
    }
}

int main(int argc, char **argv)
{
    int _sock;
    sockaddr_in server_addr;
    char buf[4096];
    if ((_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        cout << "创建sock失败" << endl;
        return -1;
    }
    cout << "socket fd is " << _sock << endl;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4567);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1");

    if (bind(_sock, (const sockaddr *)&server_addr, sizeof(sockaddr)) < 0)
    {
        cout << "绑定端口失败" << endl;
        return -1;
    }
    if (listen(_sock, 10) < 0)
    {
        cout << "倾听端口失败" << endl;
    }
    fd_set fdRead;
    timeval t = {0, 0};
    int max = _sock;

    while (true)
    {
        //每一次都进行排序，获取最大描述符的
        if(g_clients.size() > 0) {
            sort(g_clients.begin(), g_clients.end());
            max = g_clients[g_clients.size() - 1];
        }
        //每一次_sock可读 accept会新添加的， 同时如果读出的长度为0，说明断开链接需要从数组中去除
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);
        for (int i = 0; i < g_clients.size(); ++i)
        {
            FD_SET(g_clients[i], &fdRead);
        }
        int ret = select(max + 1, &fdRead, NULL, NULL, &t);
        if (ret < 0)
        {
            cout << "select发生错误" << endl;
            return -1;
        }
        if (FD_ISSET(_sock, &fdRead))
        {
            FD_CLR(_sock, &fdRead);
            sockaddr_in client_addr;
            int addrLen = sizeof(client_addr);
            int _csock = accept(_sock, (sockaddr *)&client_addr, (socklen_t *)&addrLen);
            if (_csock < 0)
            {
                cout << "接受失败" << endl;
            }
            else
            {
                g_clients.push_back(_csock);
                cout << "接受成功 "
                     << "地址是：" << inet_ntoa(client_addr.sin_addr)<< endl;
                thread t1(sendMsgToclient, _csock);
                t1.detach();
            }
        }
        for (int i = 0; i < g_clients.size(); ++i) {
            if (FD_ISSET(g_clients[i], &fdRead))
            {
                if (!processMsg(g_clients[i])) {
                    auto iter = find(g_clients.begin(), g_clients.end(), g_clients[i]);
                    if (iter != g_clients.end()) {
                        g_clients.erase(iter);
                    }
                }
            }
        }
    }
    close(_sock);
    getchar();
    return 0;
}
