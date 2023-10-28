#pragma once
#include "const.h"

#include <boost/asio.hpp>

#include <iostream>

#include <cstring>
class MsgNode
{
public:
	MsgNode(short max_length) :_total_len(max_length), _cur_len(0) {
		_data = new char[_total_len + 1];
		_data[_total_len] = '\0';
	}

	~MsgNode() {
		std::cout << "destruct MsgNode \n";
		delete[] _data;
	}
	void Clear() {
		memset(_data, 0, _total_len);
		_cur_len = 0;
	}
public:
	short _cur_len;
	short _total_len;
	char* _data;
};

class RecvNode : public MsgNode {
public:
	RecvNode(short max_len, short msg_id);
	short _msg_id;
};

class SendNode : public MsgNode {
public:
	SendNode(const char* msg,short max_len, short msg_id);
	short _msg_id;
};
