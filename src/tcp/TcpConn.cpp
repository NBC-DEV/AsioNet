#include "TcpConn.h"

namespace AsioNet
{
	// by connect
	TcpConn::TcpConn(io_ctx& ctx,IEventPoller* p) :
		m_sock(ctx),ptr_poller(p)
	{
		init();
	}

	// by accept
	TcpConn::TcpConn(TcpSock&& sock,IEventPoller* p) :
		m_sock(std::move(sock)),ptr_poller(p)
	{
		init();
	}

	TcpConn::~TcpConn()
	{
		Close();
	}

	void TcpConn::init()
	{
		NetErr ec;
		boost::asio::ip::tcp::no_delay option(true);
		m_sock.set_option(option, ec);
		m_key = 0;
		ptr_owner = nullptr;
		m_close = false;	// Ĭ�Ͽ���
	}

	// ��֤���ӽ���֮���ⲿ�����õ�conn,����Write
	// ���ӶϿ�֮���ⲿ��ʧȥconn���Ӷ��޷�write
	bool TcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}

		auto netLen = boost::asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		_lock_guard_(sendLock);
		sendBuffer.Push((const char*)(&netLen), sizeof(AN_Msg::len));
		sendBuffer.Push(data, trans);
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		return true;
	}

	void TcpConn::write_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}

		_lock_guard_(sendLock);
		sendBuffer.FreeDeatched();
		auto head = sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		//else {
		//	// ���Կ���˳���ͷ�һЩ���ͻ�����
		//  // ��ΪĿǰ���ͻ�������С�Ƕ�̬����ģ�ֻ�����󣬲�������
		//	// sendBuffer.Shrink();
		//}
	}

	void TcpConn::StartRead()
	{
		async_read(m_sock, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
			boost::bind(&TcpConn::read_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
	}

	// ͬһʱ��ֻ�����һ����/д�첽�������
	void TcpConn::read_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}

		auto netLen = *((decltype(AN_Msg::len)*)readBuffer);
		auto hostLen = boost::asio::detail::socket_ops::
			network_to_host_short(netLen);

		async_read(m_sock, boost::asio::buffer(readBuffer, hostLen),
			[self = shared_from_this()](const NetErr& ec, size_t trans) {
				if (ec)
				{
					self->err_handler(ec);
					return;
				}
				self->ptr_poller->PushRecv(self->Key(),self->readBuffer, trans);
				async_read(self->m_sock, boost::asio::buffer(self->readBuffer, sizeof(AN_Msg::len)),
					boost::bind(&TcpConn::read_handler, self, boost::placeholders::_1, boost::placeholders::_2));
			});
	}

	void TcpConn::err_handler(const NetErr& err)	// �ر�socket���������
	{
		ptr_poller->PushError(Key(), err);

		Close();	// �ر�����
	}

	// һ�����ر��ˣ����TcpConn��Ӧ��ֱ���ͷŵ�,ֻ�ܵ���һ��
	void TcpConn::Close()
	{
		{
			// �ر���֮�󣬿��ܻ��������첽�����д�������io_ctx����
			// ����Щ������ȡ���󣬶������error_handler��
			// ����ֻCloseһ�Σ���ֹ��Щ������ͣ��������Disconnect
			_lock_guard_(closeLock);
			if(m_close){	
				return;
			}
			m_close = true;
		}

		if (ptr_owner) {
			// ���ӶϿ�֮���ⲿ��Ҫ����ʧȥ��conn���ƿأ������Ļ�conn�ͻ��Զ�����
			ptr_owner->DelConn(Key());
		}
		ptr_poller->PushDisconnect(Key());// ֪ͨ�ϲ����ӹر���

		{
			// ���write_handler�������Close�ͷ���readBuffer����ô����read_handler�����readBuffer�ǲ���ȫ��
			// ����readBuffer���䱾�����'���߳�'�ܣ��Ͳ��ö������鲻�ͷ��ˣ������ټӸ���
			_lock_guard_(sendLock);
			sendBuffer.Clear();
		}
		NetErr err;
		m_sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// ֪ͨio_ctx��ȡ������m_sock���첽����
		m_key = 0;
	}

	NetKey TcpConn::Key()
	{
		if(m_key == 0){
			NetErr err;
			TcpEndPoint remote = m_sock.remote_endpoint(err);
			TcpEndPoint local = m_sock.local_endpoint(err);
			if(!err){
				m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
						| (static_cast<unsigned long long>(remote.port()) << 16)
						| static_cast<unsigned long long>(local.port()/*listen port*/);
			}
		}
		return m_key;
	}

	void TcpConn::Connect(std::string addr, unsigned short port,int retry)
	{
		TcpEndPoint ep(boost::asio::ip::address::from_string(addr.c_str()), port);
		m_sock.async_connect(ep, [self = shared_from_this(), addr,port, retry](const NetErr& ec) {
				if (ec)
				{
					self->ptr_poller->PushError(self->Key(), ec);
					if (retry > 0)
					{
						self->Connect(addr,port,retry-1);
						return;
					}
					return;
				}
				// ֻ�гɹ�������֮���ⲿ�����õ�conn
				// �����˳��ͬaccept_handler
				if (self->ptr_owner) {
					self->ptr_owner->AddConn(self);	// �ⲿ��Ӧ�ñ�����
				}
				self->ptr_poller->PushConnect(self->Key());
				self->StartRead();
			});
	}

	void TcpConn::SetOwner(ITcpConnOwner* o)
	{
		ptr_owner = o;
	}
}

