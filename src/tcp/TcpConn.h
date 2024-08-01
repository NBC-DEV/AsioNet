#pragma once

#include "../utils/AsioNetDef.h"
#include "../utils/BlockBuffer.h"
#include "../event/IEventPoller.h"

#include <unordered_map>

namespace AsioNet
{
	// doc:https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio/reference/ip__tcp/socket.html

	using TcpSock = asio::ip::tcp::socket;
	using TcpEndPoint = asio::ip::tcp::endpoint;

	struct ITcpConnOwner;	// ǰ������

	class TcpConn : public std::enable_shared_from_this<TcpConn>
	{
	public:
		TcpConn() = delete;
		TcpConn(const TcpConn&) = delete;
		TcpConn(TcpConn&&) = delete;
		TcpConn& operator=(const TcpConn&) = delete;
		TcpConn& operator=(TcpConn&&) = delete;

		// ��ʹ��shared_ptr������conn��ֱ����ջ��ʹ�ü���
		// ʵ����
		// auto conn = std::make_shared<TcpConn>(ctx, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// conn->Connect(... ...);	
		TcpConn(io_ctx& ctx, IEventPoller* p);

		// TcpServer��acceptʱʹ��
		// auto conn = std::make_shared<TcpConn>(remote, ptr_poller);
		// conn->SetOwner(ITcpConnOwner* owner);	
		// owner->AddConn(conn);	
		// ������Ϊ����ʵ��Conn��ʵ�ֵģ����Բ������AddConn���ⲿ�Լ�������
		TcpConn(TcpSock&& sock, IEventPoller* p);
		
		~TcpConn();

		void SetOwner(ITcpConnOwner*);
		
		// ��������
		bool Write(const char* data, size_t trans);

		// ��ʼ�첽����
		// �ɹ�֮�����ptr_poller->PushConnect
		void Connect(const std::string& ip, uint16_t port, int retry/*ʧ�����Դ���*/);

		// �����ر�������ӣ�ֻ�����һ��
		// ����ptr_owner->DelConn��ps��owner��Ӧ���ټ���ӵ��conn������Ȩ����Ϊ�ײ��sock�Ѿ��رգ������������в�������ʧ��
		// ����ptr_poller->PushDisconnect
		void Close();

		// ��ʼ��������
		// connect�ɹ�֮���Զ����ã�accept�����ֶ�����
		// �ɹ��������ݺ󣬵���ptr_poller->PushRecv
		void StartRead(); 
		
		// ��ȡ�Զ�addr
		TcpEndPoint Remote();

		// Ψһid
		NetKey Key();
	protected:
		void init();

		void read_handler(const NetErr&, size_t);
		void write_handler(const NetErr&, size_t);

		// ����ֱ�ӹر�����
		void err_handler();

	private:
		TcpSock m_sock;

		std::mutex m_sendLock;
		// ����һ�����ݵ����������
		static constexpr uint32_t SEND_BUFFER_SIZE = 8 * 1024;
		static const uint32_t SEND_BUFFER_EXTEND_NUM = 2;
		// ���ͻ�����
		BlockSendBuffer<SEND_BUFFER_SIZE,
			SEND_BUFFER_EXTEND_NUM> m_sendBuffer;

		// ���ջ�����
		char m_readBuffer[AN_MSG_MAX_SIZE];

		NetKey m_key;
		bool m_close;
		std::mutex m_closeLock;
		IEventPoller* ptr_poller;
		ITcpConnOwner* ptr_owner;
	};

	// ����accept,connect,disconnect�����첽�ģ�Ϊ�˷���������ӣ���������ӿ�
	// connect�ɹ���ʱ�򣬵���AddConn
	// disconnect��ʱ�򣬵���DelConn
	struct ITcpConnOwner {
		virtual void AddConn(std::shared_ptr<TcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~ITcpConnOwner() {}
	};

	// �ڲ��õ�һ��ITcpConnOwner��ʵ��
	class TcpConnMgr final:public ITcpConnOwner {
	public:
		void DelConn(NetKey) override;
		void AddConn(std::shared_ptr<TcpConn>) override;
		void Disconnect(NetKey k);

		std::shared_ptr<TcpConn> GetConn(NetKey);
		void Broadcast(const char*,size_t trans);

		~TcpConnMgr() override;
	private:
		std::unordered_map<NetKey,std::shared_ptr<TcpConn>> m_conns;
		std::mutex m_lock;
	};
}
