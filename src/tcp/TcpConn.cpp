#include "TcpConn.h"
#include <utility>	// std::move

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
		makeKey();
	}

	TcpConn::~TcpConn()
	{
		Close();
	}

	void TcpConn::init()
	{
		NetErr ec;
		asio::ip::tcp::no_delay option(true);
		m_sock.set_option(option, ec);
		m_key = 0;
		ptr_owner = nullptr;
		m_close = false;	// Ĭ�Ͽ���
	}

	// ��֤���ӽ���֮���ⲿ�����õ�conn,����Write
	// ���ӶϿ�֮���ⲿ��ʧȥconn���Ӷ��޷�write

	// ��֤���ӽ���֮���ⲿ�����õ�conn,����Write
// ���ӶϿ�֮���ⲿ��ʧȥconn���Ӷ��޷�write
	
	bool TcpConn::Write(const char* data, size_t trans)
	{
		if (trans > AN_MSG_MAX_SIZE || trans <= 0)
		{
			return false;
		}

		auto netLen = asio::detail::socket_ops::
			host_to_network_short(static_cast<decltype(AN_Msg::len)>(trans));

		_lock_guard_(m_sendLock);
		m_sendBuffer.Push((const char*)(&netLen), sizeof(AN_Msg::len));
		m_sendBuffer.Push(data, trans);
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			asio::async_write(m_sock, asio::buffer(head->buffer, head->wpos),
				std::bind(&TcpConn::write_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		return true;
	}

	void TcpConn::write_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler();
			return;
		}

		_lock_guard_(m_sendLock);
		m_sendBuffer.FreeDeatched();
		auto head = m_sendBuffer.DetachHead();
		if (head)
		{
			async_write(m_sock, asio::buffer(head->buffer, head->wpos),
				std::bind(&TcpConn::write_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		//else {
		//	// ���Կ���˳���ͷ�һЩ���ͻ�����
		//  // ��ΪĿǰ���ͻ�������С�Ƕ�̬����ģ�ֻ�����󣬲�������
		//	// m_sendBuffer.Shrink();
		//}
	}

	void TcpConn::StartRead()
	{
		async_read(m_sock, asio::buffer(m_readBuffer, sizeof(AN_Msg::len)),
			bind(&TcpConn::read_handler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}

	// ͬһʱ��ֻ�����һ����/д�첽�������
	void TcpConn::read_handler(const NetErr& ec, size_t)
	{
		if (ec)
		{
			err_handler();
			return;
		}

		auto netLen = *((decltype(AN_Msg::len)*)m_readBuffer);
		auto hostLen = asio::detail::socket_ops::
			network_to_host_short(netLen);

		asio::async_read(m_sock, asio::buffer(m_readBuffer, hostLen),
			[self = shared_from_this()](const NetErr& ec, size_t trans) {
				if (ec)
				{
					self->err_handler();
					return;
				}
				self->ptr_poller->PushRecv(self->Key(),self->m_readBuffer, trans);
				asio::async_read(self->m_sock, asio::buffer(self->m_readBuffer, sizeof(AN_Msg::len)),
					std::bind(&TcpConn::read_handler, self, std::placeholders::_1, std::placeholders::_2));
			});
	}

	void TcpConn::err_handler()	// �ر�socket���������
	{
		Close();	// �ر�����
	}

	// һ�����ر��ˣ����TcpConn��Ӧ��ֱ���ͷŵ�,ֻ�ܵ���һ��
	void TcpConn::Close()
	{
		{
			// �ر���֮�󣬿��ܻ��������첽�����д�������io_ctx����
			// ����Щ������ȡ���󣬶������error_handler��
			// ����ֻCloseһ�Σ���ֹ��Щ������ͣ��������Disconnect
			_lock_guard_(m_closeLock);
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
			// ���write_handler�������Close�ͷ���m_readBuffer����ô����read_handler�����m_readBuffer�ǲ���ȫ��
			// ����m_readBuffer���䱾�����'���߳�'�ܣ��Ͳ��ö������鲻�ͷ��ˣ������ټӸ���
			_lock_guard_(m_sendLock);
			m_sendBuffer.Clear();
		}
		NetErr err;
		m_sock.shutdown(asio::ip::tcp::socket::shutdown_both, err);
		m_sock.close(err);	// ֪ͨio_ctx��ȡ������m_sock���첽����
		m_key = 0;
	}

	NetKey TcpConn::Key()
	{
		return m_key;
	}

	void TcpConn::makeKey()
	{
			NetErr err;
			TcpEndPoint remote = m_sock.remote_endpoint(err);
			TcpEndPoint local = m_sock.local_endpoint(err);
			assert(!err);
			m_key = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
					| (static_cast<unsigned long long>(remote.port()) << 16)
					| static_cast<unsigned long long>(local.port()/*listen port*/);
	}

	void TcpConn::Connect(std::string addr, uint16_t port,int retry)
	{
		TcpEndPoint ep(asio::ip::address::from_string(addr.c_str()), port);
		m_sock.async_connect(ep, [self = shared_from_this(), addr,port, retry](const NetErr& ec) {
				if (ec)
				{
					if (retry > 0)
					{
						self->Connect(addr,port,retry-1);
						return;
					}
					return;
				}
				self->makeKey();
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

