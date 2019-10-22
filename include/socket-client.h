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

#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <memory>

class SocketClient
{
protected:
	friend class auto_ptr<SocketClient>;
	static auto_ptr<SocketClient> _singleton;
	int new_fd;
	string sockIP;
	int sockPort;

	SocketClient();
	~SocketClient();

private:
	bool open(const char *addr, int p);
	void sendString(char *str);
	void recvAck();
	void sendAck();
	bool isOpened(){ return new_fd != -1; }

public:
	static SocketClient* getInstance()
	{
		if (_singleton.get() == 0) {
			//TODO:lock
			_singleton.reset(new SocketClient());
		}
		return _singleton.get();
	}

	void setSocketIP(char* ip) {if(ip) sockIP = ip; };
	void setSocketPort(int port) { sockPort = port; };

	void goToCode(const char* filename, const char* func);
	void syncProject(const char* prjname);
	void closeSocket();
};
#endif
