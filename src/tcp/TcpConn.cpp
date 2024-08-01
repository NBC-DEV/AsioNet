#include "TcpConn.h"

namespace AsioNet
{
	TcpConn::TcpConn(io_ctx& ctx,IEventPoller* p) :
		sock_(ctx),ptr_poller(p)
	{
		init();
	}

	TcpConn::TcpConn(TcpSock&& sock,IEventPoller* p) :
		sock_(std::move(sock)),ptr_poller(p)
	{
		init();
	}

	TcpConn::~TcpConn()
	{
		close();
	}

	void TcpConn::init()
	{
		NetErr ec;
		boost::asio::ip::tcp::no_delay option(true);
		sock_.set_option(option, ec);
		key_ = 0;
		isClose = false;
	}
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
			async_write(sock_, boost::asio::buffer(head->buffer, head->wpos),
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
			async_write(sock_, boost::asio::buffer(head->buffer, head->wpos),
				boost::bind(&TcpConn::write_handler, shared_from_this(), boost::placeholders::_1, boost::placeholders::_2));
		}
		//else {
		//	// ���Կ���˳���ͷ�һЩ���ͻ�����
		//  // ��ΪĿǰ���ͻ�������С�ǻᶯ̬����ģ�ֻ�����󣬲�������
		//	// sendBuffer.Shrink();
		//}
	}

	void TcpConn::StartRead()
	{
		// if sock_ is not open,this will get an error
		async_read(sock_, boost::asio::buffer(readBuffer, sizeof(AN_Msg::len)),
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

		async_read(sock_, boost::asio::buffer(readBuffer, hostLen),
			[self = shared_from_this()](const NetErr& ec, size_t trans) {
				if (ec)
				{
					self->err_handler(ec);
					return;
				}
				self->ptr_poller->PushRecv(self->GetKey(),self->readBuffer, trans);
				async_read(self->sock_, boost::asio::buffer(self->readBuffer, sizeof(AN_Msg::len)),
					boost::bind(&TcpConn::read_handler, self, boost::placeholders::_1, boost::placeholders::_2));
			});
	}

	void TcpConn::err_handler(const NetErr& err)	// �ر�socket���������
	{
		close();	// �ر�����
	}

	// һ�����ر��ˣ����TcpConn��Ӧ��ֱ���ͷŵ�,ֻ�ܵ���һ��
	void TcpConn::close()
	{
		{
			_lock_guard_(closeLock);
			if(isClose){
				return;
			}
			isClose = true;
			// ��ֹ�Ѿ��ر���֮�󣬻����������������²�������PushDisconnect
		}

		ptr_poller->PushDisconnect(GetKey());// ֪ͨ�ϲ����ӹر���

		{
			_lock_guard_(sendLock);
			sendBuffer.Clear();
		}
		// ���write_handler�������Close�ͷ���readBuffer����ô����read_handler�����readBuffer�ǲ���ȫ��
		// ����readBuffer���䱾�����'���߳�'�ܣ��Ͳ��ö������鲻��Ҫ�ͷ��ˣ������ټӸ���
		NetErr err;
		sock_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, err);
		sock_.close(err);
		key_ = 0;
	}

	NetKey TcpConn::GetKey()
	{
		if(key_ == 0){
			NetErr err;
			TcpEndPoint remote = sock_.remote_endpoint(err);
			TcpEndPoint local = sock_.local_endpoint(err);
			if(!err){
				key_ = (static_cast<unsigned long long>(remote.address().to_v4().to_uint()) << 32)
						| (static_cast<unsigned long long>(remote.port()) << 16)
						| static_cast<unsigned long long>(local.port()/*listen port*/);
			}
		}
		return key_;
	}

	void TcpConn::Connect(std::string addr, unsigned short port)
	{
		TcpEndPoint ep(boost::asio::ip::address::from_string(addr.c_str()), port);
		sock_.async_connect(ep, boost::bind(&TcpConn::connect_handler, this, boost::placeholders::_1));
	}

	void TcpConn::connect_handler(const NetErr& ec)
	{
		if (ec)
		{
			err_handler(ec);
			return;
		}

		ptr_poller->PushConnect(GetKey());
		StartRead();
	}
}

