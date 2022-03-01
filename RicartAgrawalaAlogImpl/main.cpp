#include <iostream>
#include "Server.h"
#include "Peer.h"
#include <thread>
#include <vector>
#include <sstream>
#include "RicartAgrawalaAlgoImpl.h"
#include <fstream>

#define SharedResourcePath "/tmp/shared_resource"


void serverThreadFunction(const std::string& name, RicartAgrawalaAlgo& ricartAgrawalaAlgorithm)
{
	try{	
	LocalDomainServer server(std::string("/tmp/")+ name, [&ricartAgrawalaAlgorithm] (Message* msg){
			if (msg->type == MsgType::REQUEST)
			{
				Request *req = (Request*)msg;
				ricartAgrawalaAlgorithm.processRequest(req->timestamp, req->pId);			 
			}
			else if (msg->type == MsgType::REPLY)
			{
			  Reply* reply = (Reply*) msg;
			 std::cout<<"Reply from peerId "<<reply->pId<<std::endl;
			   ricartAgrawalaAlgorithm.processReply(reply->pId);
			}
			});
	server.run();
	}
	catch (...)
	{
		std::cout<<"Failed to start"<<std::endl;
	}

}

bool allPeersConnected(const std::vector<Peer>& peers, int currentProcessIndex)
{
	for(auto& peer:peers)
	{
		if (peer.Index() != currentProcessIndex)
		{
			if (peer.isConnected() == false)
			return false;
		}
	}
	return true;
}


bool g_allPeersConnected = false;
void addSockFdToPoll(int fd, int pollfd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
        epoll_ctl(pollfd, EPOLL_CTL_ADD, fd, &ev);
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)| O_NONBLOCK);
	std::cout<<"fd added to epoll "<<std::endl;
		
}

void PeerConnectionManagerThread( std::vector<Peer>& peers, int currentProcessIndex)
{
           int m_pollFd = epoll_create (EPOLL_SIZE);
           if (m_pollFd < 0)
           {
                   std::cout<<"Failed to create epoll "<<std::endl;
                   throw std::exception();
           }

	while(1)
	{
		for (auto& peer: peers)
		{
			if (peer.Index() == currentProcessIndex)
			{
				continue;
			}

			if (peer.isConnected() == false)
			{
				if (peer.Connect() != true)
				{
				   continue;
				}
				else
				{
			 	   std::cout<<"Connected to peer :"<< peer.name()<<std::endl;
				   //addSockFdToPoll(peer.m_sockFd, m_pollFd);
				}
			}
		}

		g_allPeersConnected = allPeersConnected(peers, currentProcessIndex);

		if (!g_allPeersConnected)
		{
			std::cout<<"One or More peers are not connected"<<std::endl;
			sleep(5);
			continue;
		}

	}

}


int main( int argc, char**argv)
{

	if (argc != 3)
	{
		std::cout<<"Invalid Arguments"<<std::endl;
		std::cout<<"./server id numberof processes"<<std::endl;
		std::cout<<"Ex: ./server 1 4"<<std::endl;
		return 0;
	}

	auto processName = std::string("P")+argv[1];

	std::cout<<"ProcessName: "<<processName<<std::endl;

	int currentProcessIndex = atoi(argv[1]);
	int totalProcesses = atoi(argv[2]);

	std::vector<Peer> m_peers;
	std::cout<<"Peers: ";

	for(int index = 0; index< totalProcesses; index++)
	{
		std::stringstream ss;
		ss<<"P"<<index;
		std::string peerName = ss.str();

		if (index != currentProcessIndex)
			std::cout<<peerName<<" ";
		m_peers.push_back(std::move(Peer(index,peerName)));
	}
	std::cout<<std::endl;

	RicartAgrawalaAlgo ricartAgrawalaAlgorithm(m_peers, currentProcessIndex);

	std::thread t1(serverThreadFunction, processName, std::ref(ricartAgrawalaAlgorithm));
	std::thread t2 (PeerConnectionManagerThread, std::ref(m_peers), currentProcessIndex);

	std::ofstream outfile;
	outfile.open(SharedResourcePath, std::ios::out|std::ios::app);

	while(true)
	{
		sleep(1);
	
		if (!g_allPeersConnected)
		{
			continue;
		}

		int inputValue;


		std::cout<<"Enter 1 to access the shared resource, 2 to release the shared resource"<<std::endl;
		std::cin>>inputValue;
		if (inputValue == 1)
		{
			bool waitingForAccess = true;
			std::cout<<"Attempting to get access to shared resource"<<std::endl;
			while (waitingForAccess)
			{
				if (ricartAgrawalaAlgorithm.hasAccessToCriticalSection())
				{
					waitingForAccess = false;
				}
				else
				{
					sleep(1);
					if (!ricartAgrawalaAlgorithm.m_requestingCriticalSection)
					{
						ricartAgrawalaAlgorithm.broadCastRequestMessage();
					}
					if (ricartAgrawalaAlgorithm.hasAccessToCriticalSection())
					{
						std::cout<<"IN CRITICAL SECTION"<<std::endl;
						outfile<<"P"<<currentProcessIndex<<" IS IN CRITICAL SECTION"<<std::endl;
						outfile<<"P"<<currentProcessIndex<< " HAS ACCESS AND IS WRITING TO THIS FILE"<<std::endl;	
					}
					else
					{
						std::cout<<"waiting for Reply from Peers " << ricartAgrawalaAlgorithm.waitingOnPeers()<<"to access CS"<<std::endl;
					}
					
				}
			}

		}
		else if (inputValue == 2)
		{
			if (ricartAgrawalaAlgorithm.hasAccessToCriticalSection())
			{
				ricartAgrawalaAlgorithm.exitCriticalSection();
				outfile<<"P"<<currentProcessIndex<<" EXITING CRITICAL SECTION"<<std::endl;
				std::cout<<"EXITING CRITICAL SECTION"<<std::endl;
			}
		}
		inputValue=0;
	}
	outfile.close();
	t1.join();
}
