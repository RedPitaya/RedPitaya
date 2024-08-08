#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>

#include "socket.h"


int createClient(SOCKET* pSock, const char *serverIp, unsigned short localPort, unsigned short serverPort){
	int result = 0;
	struct sockaddr_in localAddr;
	struct sockaddr_in serverAddr;

	if (0 == pSock)
	{
		return -1;
	}

	if ((INADDR_NONE == inet_addr(serverIp)) || (0 == serverPort))
	{
		return -1;
	}

	result = socket(PF_INET, SOCK_STREAM, 0);
	if (result < 0){
		return -1;
	}
    *pSock = result;
	memset(&localAddr, 0, sizeof(localAddr));

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(localPort);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	result = bind(*pSock, (const struct sockaddr*)&localAddr, sizeof(localAddr));
	if (result < 0)
	{
		result = close(*pSock);
		return -1;
	}

	memset(&serverAddr, 0, sizeof(serverAddr));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr.s_addr = inet_addr(serverIp);
	result = connect(*pSock, (const struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result < 0)
	{
		result = close(*pSock);
		return -1;
	}

	return 0;
}

int destroySock(SOCKET s){
	return close(s);
}

int sendTcp(SOCKET s, const char* pBuf, unsigned int len, int flags){
	unsigned int sendLen = 0;
	int result = 0;
	if (0 == pBuf) return -1;
	while (sendLen != len){
		result = send(s, &pBuf[0], len, flags);
		if (result <= 0){
			if (EINTR != result){
				return -1;
			}
		}
		else{
			sendLen += result;
		}
	}

	return sendLen;
}


int getRelativeTime(struct timeval* pTime)
{
    int result;
    struct timespec tmspc;

    if (0 == pTime){
        return -1;
    }

    result = clock_gettime(CLOCK_MONOTONIC, &tmspc);
    if (0 != result){
        return -1;
    }

    pTime->tv_sec = tmspc.tv_sec;
    pTime->tv_usec = tmspc.tv_nsec / 1000;
    return 0;
}

int recvTcp(SOCKET sock, char* pBuf, unsigned int len, int timeOutMs){
	int result = 0;
	int resSel = 0;
	unsigned int actRecvBytes = 0;
	struct timeval selectTime;
	struct timeval startTime;
	struct timeval curTime;
	struct timeval deltaTimeVal;
	long long deltaMs = 0;
	fd_set rdSet;
	fd_set edSet;

	if (0 == pBuf)
	{
		return -1;
	}

	if ((timeOutMs < 0) && (-1 != timeOutMs))
	{
		return -1;
	}

	if (0 == len)
	{
		return -1;
	}


	if (-1 == timeOutMs){
		actRecvBytes = 0;
		do
		{
			result = recv(sock, &pBuf[actRecvBytes], len - actRecvBytes, 0);
			if (result > 0)
			{
				actRecvBytes += (unsigned int)result;
			}
			else
			{
				return result;
			}
		}
		while (actRecvBytes != len);
	}
	else{
		if (getRelativeTime(&startTime)){
			return -1;
		}
		curTime = startTime;

		do
		{
			if (curTime.tv_usec >= startTime.tv_usec)
			{
				deltaTimeVal.tv_usec = curTime.tv_usec - startTime.tv_usec;
				deltaTimeVal.tv_sec = curTime.tv_sec - startTime.tv_sec;
			}
			else
			{
				deltaTimeVal.tv_usec = curTime.tv_usec + 1000000 - startTime.tv_usec;
				deltaTimeVal.tv_sec = curTime.tv_sec - 1 - startTime.tv_sec;
			}

			deltaMs = ((long long)deltaTimeVal.tv_sec * (long long)1000) + ((long long)(deltaTimeVal.tv_usec / 1000));
			if (deltaMs <= (long long)timeOutMs)
			{
				FD_ZERO(&rdSet);
				FD_ZERO(&edSet);
				FD_SET(sock, &rdSet);
				FD_SET(sock, &edSet);
				deltaMs = (long long)timeOutMs - deltaMs;
				selectTime.tv_sec = (long)(deltaMs / (long long)1000);
				selectTime.tv_usec = ((long)(deltaMs % (long long)1000)) * 1000;
				resSel = select((int)sock + 1, &rdSet, 0, &edSet, &selectTime);
				if (resSel > 0){
					result = recv(sock, &pBuf[actRecvBytes], len - actRecvBytes, 0);
					if (result > 0)
					{
						actRecvBytes += (unsigned int)result;
					}
					else
					{
						return -1;
					}
				}
				else
				{
					if (0 == resSel){
						return -2; // timeout
					}
					else{
						return -1;
					}
				}
			}
			else{
				return -2; // timeout
			}
			if (getRelativeTime(&curTime)){
				return -1;
			}
		}
		while (actRecvBytes != len);
	}
	return 0;
}


