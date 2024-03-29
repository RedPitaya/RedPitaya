#pragma once

#define SOCKET int

int createClient(SOCKET* pSock, const char *serverIp, unsigned short localPort, unsigned short serverPort);
int destroySock(SOCKET s);
int sendTcp(SOCKET s, const char* pBuf, unsigned int len, int flags);
int recvTcp(SOCKET sock, char* pBuf, unsigned int len, int timeOutMs);