#pragma once

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>

#include "AsioNetDef.h"

namespace AsioNet {
	using namespace boost::asio;
	using namespace boost::system;

	using TCPEndPoint = ip::tcp::endpoint;
// ����Խӿڵ���ʽ���ظ����棬��Ӧ�����������ʹ�ýӿڵ�ʱ����ָ�����Ч�Եõ���֤��shared_ptr?
struct AN_INTERFACE IANConn  {
	virtual bool Write(const char* acData, int iSize) = 0;
	virtual bool Read() = 0;
};



class TcpConn :boost::noncopyable {
public:
	TcpConn() = delete;
	TcpConn(io_service&);

	void Connect(std::string addr, int port);
	void Close();
	bool Write(const char* acData, int iSize);
	void Start();	// start read loop
protected:
	void connect_handler(const error_code&);
	void recv_handler(const error_code&, size_t);
	void send_handler(const error_code&, size_t);
private:
	ip::tcp::socket sock;
	char readBuffer[1024];
};

// NewTCPConn()
// NewUDPConn()
// NewKCPConn()
//class TcpConnFactory {
//
//};

}
