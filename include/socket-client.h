
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
