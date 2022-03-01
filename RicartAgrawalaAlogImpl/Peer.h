#ifndef __PEER__
#define __PEER__

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include "Message.h"
#include <string>

class Peer
{
	int index;
	std::string m_localDomainSocketName;
	std::string m_peerName;
        bool m_connected;
	bool m_requestSent;
	bool m_responseReceived;
	public:
	int m_sockFd;
	Peer(int idx,const std::string& peerName):index(idx), m_localDomainSocketName(std::string("/tmp/")+peerName), m_peerName(peerName), m_connected(false), m_requestSent(false), m_responseReceived(false)
	{
		m_sockFd = socket(AF_UNIX, SOCK_STREAM,0);
		if (m_sockFd < 0)
		{
			std::cout<<"Failed to create socket"<<std::endl;
			throw std::exception();
		}
	}


	int Index() const
	{
		return index;
	}

	bool isConnected() const
	{
		return m_connected;
	}

	const std::string& name()
	{
		return m_peerName;
	}

	bool isResponseReceived()
	{
		return m_responseReceived;
	}

	bool isRequestSent()
	{
		return m_requestSent;
	}

	bool Connect()
	{
		struct sockaddr_un  serv_addr;		
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, m_localDomainSocketName.c_str());
		int servlen = strlen(serv_addr.sun_path) + 
                 sizeof(serv_addr.sun_family);		
		auto res = connect(m_sockFd, (struct sockaddr *) &serv_addr, servlen);
		if (res < 0)
		{
			std::cout<<"Failed to connect to local domain socket "<< m_localDomainSocketName<<std::endl;
			return false;
		}
		m_connected = true;
		return m_connected;
	}

	void sendRequest(const Request& req)
	{
		m_requestSent = true;
		if (req.pId == index)
		{
			return;
		}
		auto res = write(m_sockFd, (char*)&req, sizeof(req));
		if (res<0)
			std::cout<<"Write Failed"<<std::endl;
	}

	void sendReply(const Reply& reply)
	{
		auto res = write(m_sockFd, (char*)&reply, sizeof(reply));
		if (res<0)
			std::cout<<"Write Failed"<<std::endl;
		std::cout<<"Sending Reply to Peer "<<index<<" from Id: "<<reply.pId<<std::endl;

	}


/*	void sendMessage(const Message& msg)
	{
		auto res = write(m_sockFd, (char*)&msg, sizeof(msg));
		if (res<0)
			std::cout<<"Write Failed"<<std::endl;
		std::cout<<"Sending Msg on fd: "<<m_sockFd<<std::endl;
	}
*/
	void setResponseReceived(bool flag)
	{
		m_responseReceived = flag;
	}

	void setRequestSent(bool flag)
	{
		m_requestSent = false;
	}
};

#endif
