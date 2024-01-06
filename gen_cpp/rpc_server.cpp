#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include <thrift/server/TThreadedServer.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "gen_cpp/MyService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

class MyServiceHandler : public MyServiceIf {
public:
    void myFunction(std::vector<std::string>& _return, const std::string& input) {
        _return.push_back("test");
    }
};

int main() {
    // 创建 Thrift 服务器并设置参数
    int port = 9090; // 服务器监听端口
    shared_ptr<MyServiceHandler> handler(new MyServiceHandler());
    shared_ptr<TProcessor> processor(new MyServiceProcessor(handler));
    shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
    shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // 创建 TThreadedServer 实例并启动
    TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
    cout << "Starting the server..." << endl;
    server.serve();
    cout << "Server stopped." << endl;
    return 0;
}
