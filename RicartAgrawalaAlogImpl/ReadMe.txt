OS Requirements:
1) Linux

Compiler: latest g++ 

Steps to Compile
go to Folder RicartAgrawalaAlgoImpl
Execute the comamnd
g++ -std=c++11 main.cpp Message.h Server.h Peer.h RicartAgrawalaAlgoImpl.h -o server -lpthread

Executable named server  gets created

Steps to Execute:
1) This application uses LocalDomainSockets to communicate with the Peers to send and receive the Request and Reply Messages
2) A Running instance of this executable  can be created using the command ./server processIndex totalNumOfPeers

where the argument processIndex is an integer and should be in the range 0 to (totalNumOfPeers-1)

For Example: ./server 0 3 
		where 0 is the processIndex and we have total 3 peers in this system
similarly  launch the other peers  from different terminals using the below commands
for example 
	./server 1 3 
	./server 2 3


once all the peers are connected to each other. 

we get to see the below Statement on all the terminals

"Enter 1 to access the shared resource, 2 to release the shared resource"


Once the below statement is displayed on all the terminals, that means all the peers are connected to each other and ready to access the shared resource(here a file named /tmp/shared_resource)

Now, on any terminal press 1 to request for the CS, the other processes if accessing or requessting the critical section will deffer the request.

once the criticalsection Access is granted, press 2 on the terminal to exit the CS

once 2 is presses on the terminal which had access to the CS, will release the CS and sends the pending reply messages to the other peers.
