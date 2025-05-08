#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <Windows.h>
#include <vector>
// 链接lWs2_32动态库
using namespace std;

// WebSocket 握手响应
string getWebSocketHandshakeResponse(const string &clientKey)
{
  const string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  string combined = clientKey + magic;

  // SHA1 哈希计算（实际项目可用 OpenSSL 或 Windows CryptoAPI）
  // 这里简化，直接返回固定响应（仅示例）
  return "HTTP/1.1 101 Switching Protocols\r\n"
         "Upgrade: websocket\r\n"
         "Connection: Upgrade\r\n"
         "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
}

// 解析 WebSocket 帧（仅解析文本帧）
string parseWebSocketFrame(const vector<char> &buffer)
{
  if (buffer.size() < 2)
    return "";

  // 检查是否是文本帧（OPCODE=1）
  bool isTextFrame = (buffer[0] & 0x0F) == 0x01;
  if (!isTextFrame)
    return "";

  // 获取 payload 长度
  size_t payloadLen = buffer[1] & 0x7F;
  size_t maskOffset = 2;

  if (payloadLen == 126)
  {
    payloadLen = (buffer[2] << 8) | buffer[3];
    maskOffset = 4;
  }
  else if (payloadLen == 127)
  {
    // 超长数据（暂不处理）
    return "";
  }

  // 检查是否有掩码（WebSocket 客户端必须发送掩码）
  bool hasMask = (buffer[1] & 0x80) != 0;
  if (!hasMask || buffer.size() < maskOffset + 4 + payloadLen)
  {
    return "";
  }

  // 提取掩码
  char mask[4];
  for (int i = 0; i < 4; i++)
  {
    mask[i] = buffer[maskOffset + i];
  }

  // 解码数据
  string decoded;
  for (size_t i = 0; i < payloadLen; i++)
  {
    char byte = buffer[maskOffset + 4 + i] ^ mask[i % 4];
    decoded += byte;
  }

  return decoded;
}

int main()
{
  SetConsoleOutputCP(CP_UTF8); // 字符集
  //	1	初始化
  WSADATA wsadata;
  WSAStartup(MAKEWORD(2, 2), &wsadata); // make word,你把鼠标移到WSAStartup看看参数列表,是不是就是一个word啊
  //	2	创建服务器的套接字
  SOCKET serviceSocket;
  serviceSocket = socket(AF_INET, SOCK_STREAM, 0); // socket(协议族,socket数据传输方式,某个协议)	我们默认为0,其实就是一个宏
  if (SOCKET_ERROR == serviceSocket)
  {
    cout << "套接字闯创建失败!" << endl;
  }
  else
  {
    cout << "套接字创建成功!" << endl;
  }

  //	3	绑定套接字	指定绑定的IP地址和端口号
  sockaddr_in socketAddr; // 一个绑定地址:有IP地址,有端口号,有协议族
  socketAddr.sin_family = AF_INET;
  socketAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // 代码开头第一行我们定义的宏在这就其作用啦
  socketAddr.sin_port = htons(1234);
  int bRes = bind(serviceSocket, (SOCKADDR *)&socketAddr, sizeof(SOCKADDR)); // 绑定注意的一点就是记得强制类型转换
  if (SOCKET_ERROR == bRes)
  {
    cout << "绑定失败!" << endl;
  }
  else
  {
    cout << "绑定成功!" << endl;
  }

  //	4	服务器监听
  int lLen = listen(serviceSocket, 5); // 监听的第二个参数就是:能存放多少个客户端请求,到并发编程的时候很有用哦
  if (SOCKET_ERROR == lLen)
  {
    cout << "监听失败!" << endl;
  }
  else
  {
    cout << "监听成功!" << endl;
  }

  //	5	接受请求
  while (true)
  {
    sockaddr_in revClientAddr;
    // 初始化一个接受的客户端socket
    int _revSize = sizeof(sockaddr_in);
    SOCKET recvClientSocket = accept(serviceSocket, (SOCKADDR *)&revClientAddr, &_revSize);
    if (INVALID_SOCKET == recvClientSocket)
    {
      cout << "服务端接受请求失败!" << endl;
    }
    else
    {
      cout << "服务端接受请求成功!" << endl;
    }

    //	6	发送/接受 数据
    char recvBuf[1024];
    int reLen = recv(recvClientSocket, recvBuf, 1024, 0);
    // webSocket 扩展
    string request(recvBuf, reLen);
    // 是否是websocket请求
    if (request.find("Sec-WebSocket-Key:") != string::npos)
    {
      string response = getWebSocketHandshakeResponse("test-key");
      send(recvClientSocket, response.c_str(), response.size(), 0);
      cout << "WebSocket connected" << endl;
      // 如果是websocket接收数据
      while (true)
      {
        reLen = recv(recvClientSocket, recvBuf, sizeof(recvBuf), 0);
        if (reLen <= 0)
          break;
        vector<char> frame(recvBuf, recvBuf + reLen);
        string message = parseWebSocketFrame(frame);
        if (!message.empty())
        {
          cout << "Received from client: " << message << endl;
        }
      }
    }
    else
    {
      cout << "Received:" << request << endl;
      send(recvClientSocket, "Hello from C++ Server!", 22, 0);
    }

    int sLen = send(recvClientSocket, recvBuf, reLen, 0);
    if (SOCKET_ERROR == reLen)
    {
      cout << "服务端发送数据失败" << endl;
    }
    else
    {
      cout << "服务器接受到数据:    " << recvBuf << endl
           << endl;
      system("pause");
    }

    //	7	关闭socket
    closesocket(recvClientSocket);
    closesocket(serviceSocket);
  }
  //	8	终止
  WSACleanup();

  cout << "服务器停止" << endl;
  cin.get();
  system("pause");
  return 0;
}