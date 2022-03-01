
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <functional>
#include <fcntl.h>

#include "Message.h"


struct sockaddr_un  cli_addr, serv_addr;
constexpr int EPOLL_SIZE = 1024;

class LocalDomainServer
{

	int m_sockFd;
	int m_pollFd;
        std::function<void(Message*)> m_onMsgHandlerCallback;
        std::function<void()> m_onNewConnectionCallback;

	public:
	LocalDomainServer(const std::string& name, std::function<void(Message*)> onMsgHandler):m_onMsgHandlerCallback(onMsgHandler)
	{
	   std::cout<<"Creating server: "<<name<<std::endl;

	   m_sockFd = socket(AF_UNIX, SOCK_STREAM,0);
	   if (m_sockFd<0)
	   {
		   std::cout<<"Failed to create the socket"<<std::endl;
		   throw std::exception();
	   }
	   unlink(name.c_str());	
	   struct sockaddr_un serv_addr;
	   serv_addr.sun_family = AF_UNIX;
	   strcpy(serv_addr.sun_path, name.c_str()); 
	   int servlen = strlen(serv_addr.sun_path) + 
                     sizeof(serv_addr.sun_family); 


	   auto res = bind(m_sockFd,(struct sockaddr *)&serv_addr,servlen);
	   if (res < 0)
	   {
		   std::cout<<"bind failure"<<std::endl;
		   throw std::exception();
	   }
	   listen(m_sockFd, 10);

	   m_pollFd = epoll_create (EPOLL_SIZE);
	   if (m_pollFd < 0)
	   {
		   std::cout<<"Failed to create epoll "<<std::endl;
		   throw std::exception();
	   }
	   addSockFdToPoll(m_sockFd);

	}
	
	~LocalDomainServer()
	{
		close(m_pollFd);
		close(m_sockFd);
	}

	void addSockFdToPoll(int fd)
	{
    		struct epoll_event ev;
		ev.data.fd = fd;
		ev.events = EPOLLIN;
                epoll_ctl(m_pollFd, EPOLL_CTL_ADD, fd, &ev);
                fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
	}


	void run()
	{

		struct epoll_event events[EPOLL_SIZE];

		while(1)
		{
	          int epoll_events_count = epoll_wait(m_pollFd, events, EPOLL_SIZE, -1);
 
	          if(epoll_events_count < 0) {
	            perror("epoll failure");
		    break;
		  }		

		  for(int i = 0; i < epoll_events_count; ++i)
	          {
	            int sockfd = events[i].data.fd;
	            //New user connection
	            if(sockfd == m_sockFd)
		    {
	                struct sockaddr_un client_address;
			socklen_t client_addrLength = sizeof(struct sockaddr_un);
			int clientfd = accept( m_sockFd, ( struct sockaddr* )&client_address, &client_addrLength );
                	addSockFdToPoll(clientfd);			    
		    }
		    else
		    {
			char recv_buf[2048];
			int len = recv(sockfd, recv_buf, 2048, 0);    
			if (len <= 0)
			{
			  close(sockfd);
			  break;
			}
			
			Message* msg = (Message*) recv_buf;
			m_onMsgHandlerCallback(msg);

		    }		    

		  }
		}
	}
};
