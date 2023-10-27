#pragma once
#include "const.h"
#include <boost/asio.hpp>

#include <cstring>
class MsgNode {
public:
    MsgNode(int max_len);
    virtual ~MsgNode();
    void Clear();

    int _cur_len;
    int _total_len;
    char* _data;
};

class RecvNode :public MsgNode {
public:
    RecvNode(int max_len, short msg_id);
    short _msg_id;
};

class SendNode :public MsgNode {
public:
    SendNode(const char* msg, int max_len, short msg_id);
private:
    short _msg_id;
};