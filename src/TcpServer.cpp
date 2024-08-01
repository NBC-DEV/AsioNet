#include "TcpServer.h"

namespace AsioNet
{
	// #define ERR_CHECK if(ec){std::cout << ec.message() << std::endl;return;}

	TcpServer::TcpServer(io_ctx& ctx):
		m_acceptor(ctx)
	{}

	TcpServer::~TcpServer()
	{
		NetErr err;
		m_acceptor.close(err);
	}

	void TcpServer::Serve(unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::tcp::v4(), port);
		m_acceptor.open(ep.protocol());
		m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		m_acceptor.bind(ep);
		m_acceptor.listen();

		doAccept();
	}

	// ��acceptʧ��ʱ���Զ��ͷ�TcpServer��Դ
	void TcpServer::doAccept()
	{
		m_acceptor.async_accept([self = shared_from_this()](const NetErr& ec, TcpSock cli) {
			if (ec) { return; }

			// ��client conn�Ͽ�����ʱ���Զ��ͷ�conn��Դ
			auto conn = std::make_shared<TcpConn>(std::move(cli));

			NetErr err;
			conn->poller->PushAccept(conn->sock_.remote_endpoint(err));
			conn->StartRead();

			self->doAccept();
		});
	}	
}