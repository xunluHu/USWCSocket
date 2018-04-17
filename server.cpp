#include <iostream> 

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifdef __cplusplus
}
#endif
using namespace std;

int main(int argc, char**argv) {
    int _sock;
    sockaddr_in server_addr;
    char buf[4096];
    if ((_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        cout << "创建sock失败"<< endl;
        return -1;
    }
    cout << "socket fd is "<< _sock << endl;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4567);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1");

    if (bind(_sock, (const sockaddr*)&server_addr, sizeof(sockaddr)) < 0) {
        cout << "绑定端口失败" << endl;
        return -1;
    }
    if (listen(_sock, 10) < 0) {
        cout << "倾听端口失败" << endl;
    }
    sockaddr_in client_addr;
    int addrLen = sizeof(client_addr);
    int _csock = accept(_sock, (sockaddr *)&client_addr, (socklen_t*)&addrLen);
    if (_csock < 0) {
        cout << "接受失败"<< endl;
    } 
    cout << "接受成功" << endl;
    getchar();
    return 0;
}