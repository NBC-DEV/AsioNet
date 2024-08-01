#include "./src/TcpNetMgr.h"
#include "./src/test.h"
#include "./src/utils.h"

#include <string>
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

// #include <boost/thread.hpp>

struct TestServerHandler :public AsioNet::IEventHandler {
	void AcceptHandler(AsioNet::NetKey) {};
	void ConnectHandler(AsioNet::NetKey) {};
	void DisconnectHandler(AsioNet::NetKey) {};
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) {};
	void RecvHandler(AsioNet::NetKey, const std::string&) {};
};

class TestServer :public AsioNet::IEventHandler {
public:
	TestServer() :netMgr(8, this)
	{
		netMgr.Serve(8888);
		std::cout << "listen at:8888" << std::endl;
	}
	void Update()
	{
		while (true) {
			netMgr.Update();

			// std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	void AcceptHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		auto local = AsioNet::NetKey2ServerKey(k);
		printf("accept[%d]:[%s:%d]\n", local, remote.ip.c_str(), remote.port);
	}
	void ConnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("connect to[%s:%d]\n", remote.ip.c_str(), remote.port);
	}
	void DisconnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("disconnect from[%s:%d]\n", remote.ip.c_str(), remote.port);
	}
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) override {};
	void RecvHandler(AsioNet::NetKey k, const std::string& data) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		// printf("recv from[%s:%d]:len[%d]\n", remote.ip.c_str(), remote.port, data.length());
		// echo
		bool succ = netMgr.Send(k, data.c_str(), data.length());
		if (!succ) {
			printf("echo[%s:%d]:len[%d],succ[%d]\n", remote.ip.c_str(), remote.port, data.length(), succ);
		}
	}
	void ErrorHandler(AsioNet::NetKey, const AsioNet::NetErr& err) override
	{
		std::cout << err.message().c_str() << std::endl;
	}
private:
	AsioNet::TcpNetMgr netMgr;
};

struct TestClient :public AsioNet::IEventHandler {
public:
	TestClient() :netMgr(2, this), m_conn(0), hasErr(false)
	{
		std::thread([this]() {
			while (true) {
				netMgr.Update();
				// ����ֻ��Ϊ�˲��Է��㣬���⿪�˸��̣߳�ʵ��ʹ��ʱ��Ҫ��ô��
				// ��Ȼ���߳������˻������̵߳��߼���G��
			}
			}).detach();

			netMgr.Connect("127.0.0.1", 8888, 100);
	}
	void DoTest()
	{
		while (m_conn == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		int n = AsioNet::AN_MSG_MAX_SIZE;
		std::string data;
		for (int i = 0; i < n; ++i)
		{
			data.push_back(char('a' + i % 26));
		}

		int mod = 1'0000;
		for (int trans = 1; trans <= data.length(); ++trans)
		{
			auto s = data.substr(0, trans);
			{
				_lock_guard_(lock);
				que.push(s);
			}
			bool succ = netMgr.Send(m_conn, s.c_str(), s.length());
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}
	}

	void AcceptHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		auto local = AsioNet::NetKey2ServerKey(k);
		printf("accept[%d]:[%s:%d]\n", local, remote.ip.c_str(), remote.port);
	}
	void ConnectHandler(AsioNet::NetKey k) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		printf("connect to[%s:%d]\n", remote.ip.c_str(), remote.port);
		m_conn = k;
	}
	void DisconnectHandler(AsioNet::NetKey k) override
	{
		m_conn = AsioNet::INVALID_NET_KEY;
		auto remote = AsioNet::NetKey2Addr(k);
		printf("disconnect from[%s:%d]\n", remote.ip.c_str(), remote.port);
		netMgr.Connect("127.0.0.1", 8888, 100);
	}
	void RecvHandler(AsioNet::NetKey, const char* data, size_t trans) override {};
	void RecvHandler(AsioNet::NetKey k, const std::string& data) override
	{
		auto remote = AsioNet::NetKey2Addr(k);
		{
			_lock_guard_(lock);
			if (que.front() != data)
			{
				hasErr = true;
				//printf("check error:send [%s],recv[%s]!\n",que.front().c_str(),data.c_str());
			}
			que.pop();
		}
		// printf("recv from[%s:%d]:%s\n", remote.ip.c_str(), remote.port, data.c_str());
		// echo
		// netMgr.Send(k, data.c_str(), data.length());
	}
	void ErrorHandler(AsioNet::NetKey, const AsioNet::NetErr& err) override
	{
		std::cout << err.message().c_str() << std::endl;
	}
	std::atomic<bool> hasErr;

private:
	AsioNet::TcpNetMgr netMgr;
	std::atomic<AsioNet::NetKey> m_conn;

	std::mutex lock;
	std::queue<std::string > que;
};

// ���Խ����
// �ͻ����ڴ�ʼ�ձ�����2MB���ң���ʹ����һֱ���ͳ��������򳤶ȵ����ݣ���ʱ�շ����������������ѵ�������ˣ�
// ������ÿ���ͻ��˴��ռ�ã�0.5M���ҵ��ڴ棬�ڴ����Ķ��ѻ�����Ϣ�����У��Ϳ���Ϣ�����ٶ��ж��

int main()
{
	TestServer s;
	s.Update();
	//TestClient c;
	//c.DoTest();
	//printf("client test finish:succ[%d]\n",!(c.hasErr));

	return 0;
}

/*
send[abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwx]
recv[]!

send[abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxy]
recv[abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxa]!



*/