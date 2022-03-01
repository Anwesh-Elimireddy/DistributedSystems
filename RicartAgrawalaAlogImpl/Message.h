#ifndef __MESSAGE__
#define __MESSAGE__
enum class MsgType{REQUEST,REPLY};
struct Message
{
	MsgType type;
	Message(const MsgType& t): type(t){
	}
};

struct Request:Message
{
	Request():Message(MsgType::REQUEST){}
	int timestamp;
	int pId;
};

struct Reply:Message
{
	//other fields go here 
	Reply():Message(MsgType::REPLY){}
	int pId;
};
#endif
