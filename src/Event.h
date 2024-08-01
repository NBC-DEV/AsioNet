#include "AsioNetDef.h"

namespace AsioNet
{
	/*
		�����Ҫ����Ч�ʣ����ǹ��ߵĳ����ڲ�Ҫ���ٳ���
		�����¼��������ǹ̶��ģ�����ô���֣�û��Ҫ

		class AcceptEvent {};
		class ConnectEvent {};
		class DisconnectEvent {};
		class RecvEvent {};
		struct IEvent {};
		struct AN_INTERFACE IEventPoller {
			virtual void PushEvent(IEvent*) = 0;
			virtual IEvent* PopEvent() = 0;
		};
	*/

	/*
	class TcpConn{
	private:
		IEventPoller*
	};
	*/

	struct IEventPoller {
		virtual void PushAccept(const TcpEndPoint& remote) = 0;
		virtual void PushConnect(const TcpEndPoint& remote) = 0;
		virtual void PushDisconnect(const TcpEndPoint& remote) = 0;
		virtual void PushRecv(const char* data, size_t trans) = 0;
		void (*PushAccept)(const TcpEndPoint& remote);
		void (*PushConnect)(const TcpEndPoint& remote);
		void (*PushDisconnect)(const TcpEndPoint& remote);
		void (*PushRecv)(const char* data, size_t trans);
	};

	// ��������
	// �ڲ������ӿڣ����ٿ���
	// Ҳ���÷º��������ٵ���
	struct IEventPoller1 {
		static IEventPoller1* GetInstance()
		{
			static IEventPoller1 m_poller;
			return &m_poller;
		}
		void (*PushAccept)(const TcpEndPoint& remote);
		void (*PushConnect)(const TcpEndPoint& remote);
		void (*PushDisconnect)(const TcpEndPoint& remote);
		void (*PushRecv)(const char* data, size_t trans);
	private:
		IEventPoller1()
		{
			// how can i bind to DefaultEventPoller?
			PushAccept = &(static_cast<IEventPoller*>(DefaultEventPoller::GetInstance())->PushAccept);

		}
		IEventPoller* m_poller;
	};

	struct IEventHandler {
		virtual void AcceptHandler(NetKey, const std::string& ip, unsigned short port) = 0;
		virtual void ConnectHandler(NetKey, const std::string& ip, unsigned short port) = 0;
		virtual void DisconnectHandler(NetKey, const std::string& ip, unsigned short port) = 0;
		virtual void RecvHandler(NetKey, const char* data, size_t trans) = 0;
		virtual ~IEventHandler() {}
	};

	// singleton
	class DefaultEventPoller:public IEventPoller {
	public:
		void PushAccept(const TcpEndPoint& remote) override{};
		void PushConnect(const TcpEndPoint& remote) override{};
		void PushDisconnect(const TcpEndPoint& remote) override{};
		void PushRecv(const char* data, size_t trans) override{};
		void SetHandler(IEventHandler*);

		static DefaultEventPoller* GetInstance();
		~DefaultEventPoller() {};
	protected:
		DefaultEventPoller();
	private:
		IEventHandler* m_handler;
	};

	struct DefaultEventHandler :public IEventHandler {
		void AcceptHandler(NetKey, const std::string& ip, unsigned short port) {};
		void ConnectHandler(NetKey, const std::string& ip, unsigned short port) {};
		void DisconnectHandler(NetKey, const std::string& ip, unsigned short port) {};
		void RecvHandler(NetKey, const char* data, size_t trans) {};

		static DefaultEventHandler* GetInstance()
		{
			static DefaultEventHandler m_handler;
			return &m_handler;
		}
	protected:
		DefaultEventHandler() {};
	};

#define ASIO_NET_SET_EVENT_HANDLER(h)	\
					DefaultEventPoller::GetInstance()->SetHandler(h);



	// ʹ���ߣ�ʵ��IEventHandler

}