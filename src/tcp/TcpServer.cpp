#include "TcpServer.h"

namespace AsioNet
{
	// ****************** TcpConnMgr *******************
	std::shared_ptr<TcpConn> TcpConnMgr::GetConn(NetKey k)
	{
		_lock_guard_(m_lock);
		if(conns.find(k) != conns.end()){
			return conns[k];
		}
		return nullptr;
	}
	void TcpConnMgr::DelConn(NetKey k)
	{
		_lock_guard_(m_lock);
		auto itr = conns.find(k);
		if (itr != conns.end()) {
			itr->second->Close();
			conns.erase(itr);
		}
	}
	void TcpConnMgr::AddConn(std::shared_ptr<TcpConn> conn)
	{
		_lock_guard_(m_lock);
		if(!conn){
			return;
		}
		if(conns.find(conn->Key()) == conns.end()){
			conns[conn->Key()] = conn;
		}
	}
	void TcpConnMgr::Broadcast(const char* data,size_t trans)
	{
		_lock_guard_(m_lock);
		for(auto p : conns){
			p.second->Write(data,trans);
		}
	}
	TcpConnMgr::~TcpConnMgr()
	{}
	

	// ************************************************************

	TcpServer::TcpServer(io_ctx& ctx,IEventPoller* p):
		m_acceptor(ctx),ptr_poller(p),m_key(0)
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
			auto conn = std::make_shared<TcpConn>(std::move(cli),self->ptr_poller);

			conn->SetOwner(&(self->connMgr));
			
			// ����˳���ܴ�
			// ���PushAccept֮������Write��Ҫ��֤��ʱconnMgr������
			// PushAccept��ȻҪ��PushRecv֮ǰ�����������˳��
			self->connMgr.AddConn(conn);
			self->ptr_poller->PushAccept(conn->Key());
			conn->StartRead();

			self->doAccept();
		});
	}	

	void TcpServer::Broadcast(const char* data,size_t trans)
	{
		connMgr.Broadcast(data,trans);
	}

	std::shared_ptr<TcpConn> TcpServer::GetConn(NetKey k)
	{
		return connMgr.GetConn(k);
	}

	void TcpServer::Disconnect(NetKey k)
	{
		return connMgr.DelConn(k);
	}

	ServerKey TcpServer::GetKey()
	{
		if(!m_key){
			NetErr err;
			m_key = static_cast<unsigned short>(m_acceptor.local_endpoint(err).port());
		}
		return m_key;
	}

}