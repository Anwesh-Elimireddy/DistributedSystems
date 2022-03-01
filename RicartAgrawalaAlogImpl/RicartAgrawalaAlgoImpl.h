#include "Peer.h"
#include <vector>
#include <sstream>

class RicartAgrawalaAlgo
{
	// For simplicity 
	// Assuming the number of processes in this environemnt is only three
	int localTimeStamp;
	std::vector<Peer>& m_peers; 
	int currProcessIndex;
	std::vector<int> m_requestDefferedArray;
	public:
	bool m_requestingCriticalSection;

	RicartAgrawalaAlgo(std::vector<Peer>& peers, int processIndex): m_requestingCriticalSection(false), localTimeStamp(0), m_peers(peers),currProcessIndex(processIndex)
	{
		m_requestDefferedArray.resize(peers.size());

	}

	bool hasAccessToCriticalSection()
	{
		for (auto& peer:m_peers)
		{
			if (peer.Index() != currProcessIndex)
			{
				if (peer.isResponseReceived() == false)
				return false;
			}
		}
		m_requestingCriticalSection = false;
		return true;;
	}

	std::string waitingOnPeers()
	{
		std::stringstream text;
		for (auto &peer:m_peers)
		{
			if (peer.Index() != currProcessIndex)
			{
				if (peer.isResponseReceived() == false)
				text<<"P"<<peer.Index()<<" ";
				
			}

		}

		std::string str;
		str = text.str();
		return str;
	}

	void broadCastRequestMessage()
	{
		m_requestingCriticalSection = true;
		localTimeStamp++;
		for (auto& peer: m_peers)
		{
			if (peer.isRequestSent())
			{
				continue;
			}
			Request reqMsg;
			reqMsg.pId = currProcessIndex;
			reqMsg.timestamp = localTimeStamp;
			peer.sendRequest(reqMsg);
			std::cout<<"Sending Request to Peer "<<peer.name()<<" Timestamp ("<<localTimeStamp<<","<< currProcessIndex<<")"<<std::endl;
		}
	}


	void processRequest(int timeStamp, int peerId)
	{
		std::cout<<"Received Request Message from Peer: "<<peerId<<std::endl;
		if (hasAccessToCriticalSection())
		{
			std::cout<<"This process is in CS, deffering the request"<<std::endl;
			m_requestDefferedArray[peerId] = true;
			return;
		}
		
		if (localTimeStamp > timeStamp)
		{
			// if we are here this process has requested for the critical section	
			std::cout<<"This Process has requested for the critical section, deferring the request"<<std::endl;
			m_requestDefferedArray[peerId] = true;
			return;
		}
		
		if (localTimeStamp == timeStamp)
		{
			std::cout<<"Both the timestamps are the same"<<std::endl;
			if (m_requestingCriticalSection)
			{
                 		std::cout<<"But this Process has already requested for the CS Hence Deferring the request"<<std::endl;
				m_requestDefferedArray[peerId] = true;
				return;
			}
		}

		Reply reply;
		reply.pId = currProcessIndex;
		m_peers[peerId].sendReply(reply);
		m_requestDefferedArray[peerId] = false;
		std::cout<<"Not Requesting CS Sending Reply to Peer "<< peerId<<std::endl;

	}

	void processReply(int peerId)
	{
		m_peers[peerId].setResponseReceived(true);
	}

	void exitCriticalSection()
	{
		for (auto& peer:m_peers)
		{
			if (peer.Index() != currProcessIndex)
			{
				if (m_requestDefferedArray[peer.Index()] == true)
				{
					m_requestDefferedArray[peer.Index()] = false;
					Reply reply;
					reply.pId = currProcessIndex;
					peer.sendReply(reply);
					std::cout<<"Sending out pending reply message"<<std::endl;
				}
			}
			peer.setResponseReceived(false);
			peer.setRequestSent(false);
		}

		
	}
};
