/*
1.首先修改聊天对方的IP地址
2.请首先运行服务端（TCPServer）程序，再运行客户端（TCPClient）程序：
  如配置正确服务端会收到相关连接信息。
3.连接成功后，需要由服务器端首先发起会话（输入消息并确认发送），
  客户端收到消息后才能输入消息并确认发送到服务器端。
*/
#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#pragma comment(lib,"ws2_32.lib")

using namespace std;
using std::vector;

void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
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
}

int main() {
	char send_buf[100];	//固定长度发送缓冲区和接受缓冲区
	char recv_buf[100];
	bool useless = false;
	int right_ind;	//可用Map解决登录后不再使用
	vector<SOCKET> reg_Socket;	//Socket 列表
	vector<bool> Log;
	vector<string> users = { "zzz", "123",
							"ccc", "456"};
	
	//定义服务端套接字，接受请求套接字
	SOCKET s_server;
	SOCKADDR_IN server_addr;	//服务端地址
	memset(&server_addr, 0, sizeof(server_addr));

	initialization();
	s_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s_server == INVALID_SOCKET) {
		cout << "socket  error:" << GetLastError() << endl;
		return 0;
	}
	server_addr.sin_family = AF_INET;	//填充服务端信息
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	//本地IP
	server_addr.sin_port = htons(5010);

	if (bind(s_server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR))== SOCKET_ERROR) {
		cout << "服务端socket绑定失败！" << endl;
		WSACleanup();
	}
	else {
		cout << "服务端socket绑定成功！" << endl;
	}
	//设置套接字为监听状态
	if (listen(s_server, 5) == SOCKET_ERROR) {
		cout << "设置监听状态失败！" << endl;
		WSACleanup();
		return 0;
	}
	else {
		cout << "设置监听状态成功！" << endl;
		cout << "服务端正在监听连接，请稍候...." << endl;
	}

	fd_set readSet;	//定义一个读（接受消息）的集合
	FD_ZERO(&readSet);//初始化集合
	FD_SET(s_server, &readSet);
	while (true) {
		fd_set tmpSet;	//定义一个临时的集合
		FD_ZERO(&tmpSet);	//初始化集合
		tmpSet = readSet;	//利用select选择出集合中可以读写的多个套接字
		int ret = select(0, &tmpSet, NULL, NULL, NULL);		//最后一个参数为NULL，一直等待，直到有数据过来
		if (ret == SOCKET_ERROR) {
			continue;
		}		
		for (int i = 0; i < tmpSet.fd_count; ++i) {	//筛选出来正确的tmpSet可以发送或者接收的socket
			//获取到套接字
			SOCKET s = tmpSet.fd_array[i];
			if (s == s_server) {
				SOCKET c = accept(s, NULL, NULL);
				if (readSet.fd_count < FD_SETSIZE) {  //fd_set集合最大值为64					
					FD_SET(c, &readSet);  //往集合中添加客户端套接字
					cout << c << "已进入聊天室！" << endl;
					//将Socket添加至vector,添加一个登录量
					reg_Socket.push_back(c);
					Log.push_back(false);
					strcpy_s(send_buf, "请进行登录"); 
					send(c, send_buf, 100, 0);
				}
				else
					cout << "达到客户端容量上线！" << endl;
			}
			else {	//一定是客户端				
				ret = recv(s, recv_buf, 100, 0);	//接收客户端的数据
				if (ret == SOCKET_ERROR || ret == 0) {
					closesocket(s);
					FD_CLR(s, &readSet);	//从集合中去除
					cout << s << "离开聊天室！" << endl;
				}
				else {
					cout << s << "说：" << recv_buf << endl;	
					//(&recv_buf[0], &recv_buf[strlen(recv_buf)]);	//将字符数组转为string
					/*cout << Log[Log.size() - 1] << endl;
					cout << Log.size() - 1 << endl;*/
					if (!Log[Log.size()-1]) {	//无法满足同时登录					
						if (useless) {
							if (recv_buf == users[right_ind+1]) {
								send(s, "YES", 10, 0);
								useless = false;
								Log[Log.size() - 1] = true;
							}
							else {
								send(s, "NO", 100, 0);
								cout << s << "密码错误" << endl;
							}
						}
						else {
							for (int ind = 0; ind < users.size(); ind += 2) {
								if (recv_buf == users[ind]) {
									send(s, "YES", 100, 0);
									useless = true;
									right_ind = ind;
									break;
								}
							}
							if (!useless) {
								send(s, "NO", 100, 0);
								cout << s << "用户名不存在" << endl;
							}							
						}						
						continue;
					}
					//消息转发
					int use_ind = 0;
					string tmp = recv_buf;					
					for (SOCKET t:reg_Socket) {				
						if (t == s) {
							tmp = users[use_ind] + ":" + tmp;
							break;
						}											
						use_ind += 2;
					}
					strcpy_s(recv_buf, tmp.c_str());	//转为char[]
					for (SOCKET t : reg_Socket) {						
						send(t, recv_buf, 100, 0);
					}
				}
			}
		}
	}
	//关闭套接字
	closesocket(s_server);
	//释放DLL资源
	WSACleanup();
	return 0;
}
