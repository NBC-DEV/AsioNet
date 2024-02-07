#pragma once

#include "AsioNetDef.h"
#include "event/IEventHandler.h"
#include "event/DefaultEventDriver.h"
#include "tcp/TcpServer.h"

#include <map>

namespace AsioNet
{

    /*
                    事件处理器
    TcpNetMgr------>DefaultEventDriver---->Handler
      |     \          ^
      v      \         |
    TcpServer \    IEventPoller
      \        |       ^
       \       v      /
        -->TcpConn----
    
    *注意*：
    1.Update会以顺序的方式取走并处理完所有的事件才返回
    2.默认的事件处理器,优先为了效率考虑：
      将调用签名为 void RecvHandler(NetKey, const char *data, size_t recv) 的接口函数。
      传递给RecvHandler的参数为内部存储的底层指针和数据大小，不做额外的数据拷贝。
      使用者如果对该指针进行了写操作/越界访问，是很危险的，请注意。
    3.你可以通过修改DefaultEventDriver::PopOne中对EventType::Recv事件的处理方式来修改上述行为
    // Encoder,Decoder,Router放在业务层私以为更加合理，遂不在网络层做处理
    
    */
    
    class TcpServerMgr{
    public:
      std::shared_ptr<TcpServer> GetServer(ServerKey);
      void AddServer(std::shared_ptr<TcpServer>);
      ~TcpServerMgr();
    private:
      std::mutex m_lock;
      std::map<ServerKey,std::shared_ptr<TcpServer>> servers;
    };

    class TcpNetMgr {
    public:
        TcpNetMgr() = delete;
        TcpNetMgr(const TcpNetMgr&) = delete;
        TcpNetMgr& operator=(const TcpNetMgr&) = delete;

        TcpNetMgr(size_t th_num/*线程数量*/,IEventHandler*);
        ~TcpNetMgr();

        // 直到把当前的请求全部处理完才结束，占用调用者的线程资源
        void Update();
        ServerKey Serve(unsigned short port/*,options*/);
        void Broadcast(ServerKey, const char* data, size_t trans);

        // retry：连接失败后的重试次数
        void Connect(std::string ip, unsigned short port,int retry = 1/*,options*/);       
        void Disconnect(NetKey);
        bool Send(NetKey, const char* data, size_t trans);

    private:
        io_ctx ctx;
        std::vector<std::thread> thPool;
        DefaultEventDriver m_poller;
        TcpConnMgr connMgr;
        TcpServerMgr serverMgr;
    };

}