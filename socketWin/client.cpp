#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<windows.h>
#include<WinSock2.h>
#include<stdio.h>
#include <string>
#include <iostream>
#include <thread>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

bool exitFlag = false;
void readFromServer(SOCKET _sock) {
	char receBuf[1024];
	while (!exitFlag) {
		memset(&receBuf, 0, sizeof(receBuf));
		int len = recv(_sock, (char*)&receBuf, sizeof(receBuf), 0);
		DWORD dBufSize = MultiByteToWideChar(CP_UTF8, 0, receBuf, len, NULL, 0);

		wchar_t * dBuf = new wchar_t[dBufSize];
		wmemset(dBuf, 0, dBufSize);

		//进行转换
		int nRet = MultiByteToWideChar(CP_UTF8, 0, receBuf, len, dBuf, dBufSize);
		if (nRet < 0) {
			cout << "转换失败,停止读数据" << endl;
			break;
		}
		if (len > 0 && !exitFlag) {
			wcout.imbue(locale("chs"));
			wcout << L"服务器：" << dBuf << endl;
		}
		delete dBuf;
	}
}

int main()
{
	string serveAddress = "192.168.1.110";
	//启动Windows socket 2.x环境
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
	//------------
	//-- 用Socket API建立简易TCP客户端
	// 1 建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，建立Socket失败...\n");
	}
	else {
		printf("建立Socket成功...\n");
	}
	// 2 连接服务器 connect
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr(serveAddress.c_str());
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("错误，连接服务器失败...\n");
	}
	else {
		printf("连接服务器成功...\n");
	}

	thread t1(readFromServer, _sock);
	char msg[1024];
	while (!exitFlag) {
		memset(&msg, 0, sizeof(msg));
		cin.getline(msg, sizeof(msg));
		if (memcmp("exit", &msg, sizeof("exit")) == 0) {
			exitFlag = true;
		}
		int n = send(_sock, (const char*)&msg, sizeof(msg), 0);
	}
	t1.join();
	// 7 关闭套节字closesocket
	closesocket(_sock);
	//清除Windows socket环境
	WSACleanup();
	printf("bye");
	getchar();
	return 0;
}