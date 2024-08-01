#pragma once

#include "AsioNetDef.h"
#include "BlockBuffer.h"
#include <queue>

namespace AsioNet
{
	enum class EventType
	{
		Accept,
		Connect,
		Disconnect,
		Recv,
	};
	struct NetEvent
	{
		NetKey key;
		EventType type;
	};

    struct IEventHandler
	{
		virtual void AcceptHandler(NetKey) = 0;
		virtual void ConnectHandler(NetKey) = 0;
		virtual void DisconnectHandler(NetKey) = 0;
		virtual void RecvHandler(NetKey, const char *data, size_t trans) = 0;
		virtual void RecvHandler(NetKey, const std::string &) = 0;

		virtual ~IEventHandler() {}
	};

	// ��������
	// �ڲ����������ӿڣ�Ҳ���÷º��������ٿ���
    // �ڲ��Ѿ�ʵ����һ��Poller����ֻ�Ǹ��ⲿ�Զ����õ�
    struct IEventPoller
	{
		virtual void PushAccept(NetKey k) = 0;
		virtual void PushConnect(NetKey k) = 0;
		virtual void PushDisconnect(NetKey k) = 0;
		virtual void PushRecv(NetKey k, const char *data, size_t trans) = 0;

		virtual ~IEventPoller(){}
	};

	const unsigned int DEFAULT_POLLER_BUFFER_SIZE = AN_MSG_MAX_SIZE;
	const unsigned int DEFAULT_POLLER_BUFFER_EXTEND_NUM = 2;
	class DefaultEventPoller : public IEventPoller
	{
	public:
		void PushAccept(NetKey k);
		void PushConnect(NetKey k);
		void PushDisconnect(NetKey k);
		void PushRecv(NetKey k, const char *data, size_t trans);

		DefaultEventPoller(IEventHandler*);
		~DefaultEventPoller(){};

	protected:
	private:
		IEventHandler *ptr_handler;
		std::mutex m_lock;
		std::queue<NetEvent> m_events;
		BlockBuffer<DEFAULT_POLLER_BUFFER_SIZE,
					DEFAULT_POLLER_BUFFER_EXTEND_NUM> m_recvBuffer;
	};

}