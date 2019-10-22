/*
** This file is a part of miniStudio, which provides a WYSIWYG UI designer
** and an IDE for MiniGUI app developers.
**
** Copyright (C) 2010 ~ 2019, Beijing FMSoft Technologies Co., Ltd.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#ifndef WIN32
//linux
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#else
//win32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "wininet.lib")

#endif

#include <string>
#include <memory>

using namespace std;

#define BUFFSIZE    2048

#include "socket-client.h"

#include "log.h"

auto_ptr<SocketClient> SocketClient::_singleton;

SocketClient::SocketClient()
{
    new_fd = -1;
#ifdef WIN32
	{
		WORD wVersionRequested;
		WSADATA wsaData;
		wVersionRequested = MAKEWORD(2, 2);
		WSAStartup(wVersionRequested, &wsaData);
	}
#endif
};

SocketClient::~SocketClient()
{
	closeSocket();

#ifdef WIN32
	WSACleanup();

#endif
}

bool SocketClient::open(const char *addr, int p)
{
	struct sockaddr_in  target_addr;
	struct hostent      *he;

	if(addr == NULL || *addr == '\0')
		return false;

	if ((he = gethostbyname(addr)) == NULL)
	{
		perror("gethostbyname");
		return false;
	}

    if ((new_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("socket");

    target_addr.sin_family   = AF_INET;
    target_addr.sin_port     = htons(sockPort);
    target_addr.sin_addr     = *((struct in_addr *) he->h_addr);

#ifdef WIN32
	memset(&(target_addr.sin_zero), 0, 8);
#else
    bzero(&(target_addr.sin_zero), 8);
#endif

    if (connect(new_fd, (struct sockaddr *)&target_addr,
                sizeof(struct sockaddr)) == -1) {
        //perror("connect server");
        closeSocket();
        return false;
    }

    return true;
}

/*
 * sample:
 * GUISEND\r\n
 * type:1\r\n
 * prjname:test\r\n\r\n
 **/
void SocketClient::syncProject(const char* prjname)
{
    char header[]   = "GUISEND";
    char typesect[] = "type:";
    char prjsect[] = "prjname:";
    int  /*packlen,*/ type = 1;
    //char *pack;
		char pack[1024*4];

    if (!prjname)
    	return;

    if(!isOpened()) {
    	if (!open(sockIP.c_str(), sockPort))
	    	return;
    }
    /* 8 for crlf, 2 for type value and '\0' */
    //packlen = 6 + strlen(header) + strlen(typesect) + 1
			//	+ strlen(prjsect) + strlen(prjname) + 1;
    //pack = (char *)calloc (1, packlen);
    sprintf (pack, "%s\r\n%s%d\r\n%s%s\r\n\r\n",
    		header, typesect, type, prjsect, prjname);

    sendString (pack);
    //free(pack);
}

/*
 * sample:
 * GUISEND\r\n
 * type:0\r\n
 * file:test.java\r\n
 * key:int MiniGUIMain\r\n\r\n
 **/
void SocketClient::goToCode(const char* filename, const char* func)
{
    char header[]   = "GUISEND";
    char typesect[] = "type:";
    char filesect[] = "file:";
    char funcsect[] = "key:";
    int  packlen, type = 0;
    char *pack;

    if (!func || !filename)
    	return;

    if(!isOpened()) {
    	if (!open(sockIP.c_str(), sockPort))
	    	return;
    }

    /* 10 for crlf, 1 for type value, 1 for '\0' */
    packlen = 10 + strlen(header) + strlen(typesect) + 1
			+ strlen(filesect) + strlen(filename)
            + strlen(funcsect) + strlen(func) + 1;

    pack = (char *)calloc (1, packlen);

    sprintf (pack, "%s\r\n%s%d\r\n%s%s\r\n%s%s\r\n\r\n",
                    header, typesect, type,
                    filesect,filename,
                    funcsect, func);

    sendString (pack);
    free(pack);
}

void SocketClient::closeSocket()
{
#ifdef WIN32
	shutdown(new_fd, SD_BOTH);
#else
    shutdown(new_fd, SHUT_RDWR);
#endif
    new_fd = -1;
};

/*===================== private =========================*/

// send a string to the socket
void SocketClient::sendString(char *str)
{
    if (send(new_fd, (char *) str, strlen(str), 0) == -1) {
        perror("send");
        return;
    }

	//printf("client: sending string '%s'\n", str);
	recvAck();
	sendAck();

};

void SocketClient::recvAck()
{
	char temp[1];
	int total = 0;
    int ret;

	while (total < 1) {

	   ret = recv(new_fd, temp, 1, 0);
       if (ret == 0) {
           closeSocket();
           return;
       }
	   total += ret;
    }
};

void SocketClient::sendAck()
{
	char tmp[1];
	tmp[0] = 42;

	send(new_fd, tmp, 1, 0);
};
