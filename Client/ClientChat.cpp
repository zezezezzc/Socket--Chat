#include <iostream>
#include <winsock2.h>
#include <thread>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

using namespace std;

void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;	//WSADATA变量,包含windows socket执行的信息
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息
}

bool Login(SOCKET s)
{
	int send_len = 0;
	int recv_len = 0;
	char s1[100], s2[100];
	cout << "用户名:";
	cin >> s1;
	send_len = send(s, s1, 100, 0);
	recv_len = recv(s, s1, 100, 0);
	if (recv_len == SOCKET_ERROR)
		cout << "Server接收失败！" << endl;
	else
		cout << s1 << endl;
	string yes = "YES";
	if (s1 == yes) {	//s1 == "YES" 与常量比较错误
		cout << "请输入密码:";
		cin >> s2;
		send_len = send(s, s2, 100, 0);
		recv_len = recv(s, s2, 100, 0);
		if (recv_len == SOCKET_ERROR) {
			cout << "Server接收失败！" << endl;
			cerr << "WRONG" << endl;
			return false;
		}
		else {
			if (s2 == yes) {
				cout << "登录成功" << endl;
				return true;
			}
		}
	}
	else {
		cout << "用户名不存在" << endl;
		return false;
	}
}

DWORD WINAPI CreateSendMegThread(LPVOID lpParameter)
{
	SOCKET s_send = (SOCKET)lpParameter;
	cin.ignore();
	while (true)
	{
		char meg_send[100] = {};
		int send_len = 0;
		cout << "请输入发送信息:";
		cin.getline(meg_send, 100);
		send_len = send(s_send, meg_send, 100, 0);
		if (send_len == SOCKET_ERROR) {
			cout << "发送失败！" << endl;
			continue;
		}
	}
	return 0;
}

DWORD WINAPI CreateRecvMegThread(LPVOID lpParameter)
{
	SOCKET s_recv = (SOCKET)lpParameter;
	while (true)
	{
		char meg_recv[100] = {};
		int recv_len = 0;
		recv_len = recv(s_recv, meg_recv, 100, 0);
		if (recv_len == SOCKET_ERROR) {
			cout << "接受失败！" << endl;
			continue;
		}
		else {
			cout << endl << meg_recv << endl;
		}
	}
	return 0;
}

int main() {
	SOCKET s;	//定义服务端socket，接受请求套接字
	SOCKADDR_IN server_addr;//服务器端地址
	int recv_len = 0;
	char recv_buf[100] = {};
	initialization();
	
	server_addr.sin_family = AF_INET;	//填充服务端信息
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5010);		//端口设置

	//创建socket
	s = socket(AF_INET, SOCK_STREAM, 0);

	if (connect(s, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		cout << "服务器连接失败！" << endl;
		WSACleanup();
		return -1;
	}
	else
		cout << "服务器连接成功！" << endl;

	recv_len = recv(s, recv_buf, 100, 0);
	if (recv_len == SOCKET_ERROR)
		cout << "Server接收失败！" << endl;
	else
		cout << recv_buf << endl;

	bool sign = Login(s);

	if (sign) {
		CreateThread(nullptr, 0, CreateRecvMegThread, (LPVOID)s, 0, nullptr);
		CreateThread(nullptr, 0, CreateSendMegThread, (LPVOID)s, 0, nullptr);
		while (true) {
			;
		}
	}
	//关闭套接字
	closesocket(s);
	//释放DLL资源
	WSACleanup();
	return 0;
}
