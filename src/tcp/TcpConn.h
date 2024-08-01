#pragma once

#include "../include/AsioNetDef.h"
#include "../include/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	struct ITcpConnOwner;	// ǰ������

	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html
	constexpr unsigned int SEND_BUFFER_SIZE = 8 * 1024;// AN_MSG_MAX_SIZE;
	const unsigned int SEND_BUFFER_EXTEND_NUM = 2;
	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
		// ����ϣ�friend
		friend class TcpServer;
	public:
		TcpConn() = delete;
		TcpConn(const TcpConn&) = delete;
		TcpConn& operator=(const TcpConn&) = delete;


		// ʹ��˵���������shared_ptrʹ�ã�ֱ����ջ��ʹ�ü���
		// auto conn = std::make_shared<TcpConn>(ctx, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// conn->Connect(... ...);	
		// �첽���ӳɹ�֮�󣬻��Զ�����owner->AddConn
		// �벻Ҫ��û�����ӳɹ�ʱ���Լ�����owner->AddConn,�߼���Ҳ����ȷ
		TcpConn(io_ctx& ctx, IEventPoller* p);

		// TcpServer��acceptʱר�õĹ���
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// owner->AddConn(conn);	// ������Ϊ�����Ѿ��ɹ��ˣ�����ֱ��Add�Ǻ����
		TcpConn(TcpSock&& sock, IEventPoller* p);
		
		~TcpConn();

		// ����������Ǳ����
		// ���磺�������Ҫ�������ӣ���ȫ����״̬���������������ȫ����Ҫ,ֻ��������������
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->StartRead()
		void SetOwner(ITcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		void Connect(std::string addr, unsigned short port, int retry);

		// �����ر�������ӣ�һ���ر����ӣ��ͻ����owner->DelConn
		// owner��Ӧ���ټ���ӵ��conn������Ȩ����Ϊ�ײ��sock�Ѿ��رգ������������в�������ʧ��
		// ֻ�����һ��
		void Close();

		NetKey Key();
	protected:
		void init();

		// ֻ����connect/accept�ɹ���֮��ʹ��
		void StartRead(); 

		void read_handler(const NetErr&, size_t);
		void write_handler(const NetErr&, size_t);

		void err_handler(const NetErr&);

	private:
		TcpSock m_sock;
		std::mutex sendLock;
		// ȱ�㣺
		// ��������С�޸����������±���,
		// ÿ��sendBuffer���ֿ��������ֲ����ڴ�ص�Ч�ʣ����������д
		BlockSendBuffer<SEND_BUFFER_SIZE,
			SEND_BUFFER_EXTEND_NUM> sendBuffer;
		char readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;
		bool m_close;
		std::mutex closeLock;
		IEventPoller* ptr_poller;
		ITcpConnOwner* ptr_owner;
	};

	struct ITcpConnOwner {	// Ϊ�˹������Ӷ����
		virtual void AddConn(std::shared_ptr<TcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~ITcpConnOwner() {}
	};
}
