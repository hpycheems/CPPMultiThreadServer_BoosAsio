#pragma once

#include "const.h"
#include "MsgNode.h"

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <iostream>
#include <memory>
#include <queue>
#include <mutex>

using boost::asio::ip::tcp;

class CServer;

class CSession :public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& io_context, CServer* server);
    ~CSession();
    std::string& GetUuid();
    tcp::socket& GetSocket();
    void Close();
    void Start();
    std::shared_ptr<CSession> SessionSelf();
    void Send(char* msg, int max_len, short msg_id);
    void Send(std::string msg, short msg_id);
private:
    void HandleRead(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session);
    void HandleWrite(const boost::system::error_code& ec, std::shared_ptr<CSession> session);
    void HandleReadHead(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session);
    void HeadleReadMsg(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<CSession> session);

    std::mutex _send_lock;
    tcp::socket _socket;
    char _data[MAX_LENGHT];
    std::string _uuid;
    CServer* _server;
    std::queue<std::shared_ptr<SendNode>> _send_queue;
    std::shared_ptr<MsgNode> _recv_head_node;
    std::shared_ptr<RecvNode> _recv_msg_node;
    bool _b_head_parse;
    bool _b_close;
    //´®ÐÐ
    //boost::asio::strand<boost::asio::io_context::executor_type> _strand;
};

class LogicNode {
    friend class LogicSystem;
public:
    LogicNode(std::shared_ptr<CSession> session, std::shared_ptr<RecvNode> recvNode) :_session(session), _recvNode(recvNode) {}
private:
    std::shared_ptr<CSession> _session;
    std::shared_ptr<RecvNode> _recvNode;
};