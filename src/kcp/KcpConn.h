#pragma once

#include <ikcp.h>

#include "../utils/AsioNetDef.h"
#include "../utils/BlockBuffer.h"
#include "../event/IEventPoller.h"

namespace AsioNet
{
	using UdpEndPoint = asio::ip::udp::endpoint;
	using UdpSock = asio::ip::udp::socket;
	
	using KcpKey = uint64_t;

	const size_t AN_KCP_BUFFER_SIZE = 4096;

	struct IKcpConnOwner;	// 前向声明
	// doc:https://github.com/libinzhangyuan/asio_kcp

	// ikcp_allocator:可以考虑接管内存管理
	class KcpConn : public std::enable_shared_from_this<KcpConn>
	{
	public:
		KcpConn() = delete;
		KcpConn(const KcpConn&) = delete;
		KcpConn(KcpConn&&) = delete;
		KcpConn& operator=(const KcpConn&) = delete;
		KcpConn& operator=(KcpConn&&) = delete;

		KcpConn(uint32_t id,io_ctx& ctx, IEventPoller* p);

		~KcpConn();

		void SetOwner(IKcpConnOwner*);
		
		bool Write(const char* data, size_t trans);

		void Close();

		void Start();

		NetKey Key();
		
	protected:
	    static int kcpOutPutFunc(const char *buf, int len,ikcpcb *kcp, void *user);

		void init();

		void kcpUpdate();

		void startRead();

		void err_handler();

	private:
        // kcp-go中的svr，多个kcp依赖在一个udpsock上
		// 客户端只会知道服务器的一个addr，如果使用多个udpsock显然是不合理的
		// 并且既然使用kcp了，那么链接显然不不会有太多个
		std::shared_ptr<UdpSock> m_sock;

        ikcpcb *m_kcp = nullptr;
		asio::high_resolution_timer m_updater;
		std::mutex m_kcpLock;

		UdpEndPoint m_sender;	// 发送用
		
		// used only in svr mode
		UdpEndPoint m_tempRecevier;	// 接收用

		// BlockSendBuffer<1024,2> m_sendBuffer;
        // 用于接受kcp协议的buffer，kcp协议经过分片处理，不需要很大
		// 这里看4096比较顺眼就用这个数字了
		char m_kcpBuffer[AN_KCP_BUFFER_SIZE];
		char m_readBuffer[AN_MSG_MAX_SIZE];
		NetKey m_key;

		IEventPoller* ptr_poller;
		IKcpConnOwner* ptr_owner;
	};

	struct IKcpConnOwner {	// 为了管理连接而设计
		virtual void AddConn(std::shared_ptr<KcpConn>) = 0;
		virtual void DelConn(NetKey) = 0;
		virtual ~IKcpConnOwner() {}
	};
}
