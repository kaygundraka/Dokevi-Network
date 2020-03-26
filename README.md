# Dokevi-Network
> The Dokevi-Network helps you make c++ network game client and server easier.

![version](https://img.shields.io/badge/alpha-v1.0.0-blue.svg)
![language](https://img.shields.io/badge/language-c%2B%2B-green.svg)
![platform](https://img.shields.io/badge/platform-windows-brightgreen.svg)

The Dokevi-Network includes many utils like network(client/server/packet), structure, os(thread/exception/log), interface.
The Dokevi-Network based on windows-platform. Because this project uses powerful-network function likes iocp. This library provides multi-thread iocp based tcp server and independence tcp-client. And then, it helps you to use some utils as structures and interfaces easier.

This project include spdlog library. (MIT License Open Source Library)
> https://github.com/gabime/spdlog

## Features

+ Provide windows-iocp based tcp server. (React Type Server)
+ Provide independece abstract network client.
+ Provide some utils as structures and interfaces.
+ Provide tiny and simple c++ network game framework.
+ Logs, dumps and records make it easy to find bugs.

## Installation

Maybe using cmakes.
> Wait for moment.

## Simple Guide

### Step 0 : Install library and build project

 The Dokevi-Network use iocp tcp interface in windows platform. We provide visual studio project using cmake. If you build the project, you can get header files and static lib files. You have to link library to your project. 

### Step 1 : How to create connection class?

 If you want to server, just you create dokevi-network server instance. But connection as game client is made inherited class in library. You have to make your connection class using abstract connection class.

 Look at the code below.

> This library code is always writed on Dokevi namespace.

```cpp
class NetworkClient abstract
{
protected:
	TCPSocket* _socket;

public:
	NetworkClient(TCPSocket* inSocket) : _socket(inSocket) {}
		
	const TCPSocket* GetSocket() { return _socket; }
	~NetworkClient();

	virtual void ConnectedHandler() = 0;
	virtual void DisconnectedHandler() = 0;
	virtual void PacketHandler(unsigned int inSize, void* inData) = 0;
};
```

> You can find member value by '_' prefix keyword. Every member values use '_' keyword.
> Also, function parameters alway use 'in' prefix keyword.

 The NetworkClient is declared on NetworkClient header file. This class works like connection. When any client is connected to your server, The network thread create The NetworkClient class instance and call abstract ConnectedHandler function.

 In the same way, DisconnectedHandler called. When your server is received packet, The NetworkClient process packet using your inherited handler.

 You can make connection easier using NetworkClient interface. Also, the dokevi-server creates connections separately by port numbers. The server can handle many kinds of connection in parallel.

### Step 2 : How to launch server?

 If you want to use the dokevi-network, It is needed to initialize. First, You call function - NetworkUtils::InitializeNetwork(). Because the dokevi-network use windows network library. You can use by including InterfaceFunction.h file.

```cpp
#include <InterfaceFunction.h>

using namespace Dokevi;

void main()
{
    DokeviUtils::InitializeNetwork();

    // ...
}
```

 And then, The dokevi-network framework use many managers inside. You should initialize managers.
It is simple. Just write code below.

> Dokevi::DokeviUtils::IntializeManagers(".\\settings.ini");

 The parameter string is server config file (as ini-type file) address. It is used by config manager. If you want to input any options, you should write options in ini file. You can use options in code by config manager easier.

 Also, When the server or client program is finished, you must write to below code.

> Dokevi::DokeviUtils::ReleaseMembers();

 It release the manager's resources.

 The base code likes below code.

```cpp
#include <InterfaceFunction.h>

using namespace Dokevi;

void main()
{
    DokeviUtils::InitializeNetwork();

    DokeviUtils::InitializeManagers(".\\settings.ini");

    // some codes

    DokeviUtils::ReleaseManagers();
}
```

 Now, We almost complete ready for create server. But we not yet declare connection instance alloc function.
You should ready to declare connection alloc function seperatly by ports. Look at the next example code.

```cpp
// 1. Declare example alloc function
void* CreateGameClient(TCPSocket* inSocket) // Must be same function-declare type
{
    return new MyCode::GameClient(inSocket); // this class inherit NetworkClient class
}

// 1-1. In same way, declare alloc function
void* CreateWebToolClient(TCPSocket* inSocket)
{
    return new MyCode::WebToolClient(inSocket); // this class inherit NetworkClient class
}
```

 The preparations are complete. you create server instance and run with connection alloc handler.
> If you use config managers, it helps you to use options eaiser.

```cpp
#include <InterfaceFunction.h>

using namespace Dokevi;

// declar connection alloc functions

void main()
{
    DokeviUtils::InitializeNetwork();

    DokeviUtils::InitializeManagers(".\\settings.ini");

    DokeviServer server;

    const int iocpWorkerNum = ConfigMgr->GetInt("System"/*section*/, "IOThread"/*value*/);
    const int gamePort = ConfigMgr->GetInt("System"/*section*/, "GamePort"/*value*/);
    const int toolPort = ConfigMgr->GetInt("System"/*section*/, "ToolPort"/*value*/);

    EServerRunResult result = server.
        SetWorkerThead(iocpWorkerNum).
        SetConnectionInfo(gamePort, &CreateGameClient).
        SetConnectionInfo(toolPort, &CreateToolClient).
        Run();

    // result error check ... 

    // loop game logic or sleep.

    DokeviUtils::ReleaseManagers();
}
```

 The DokeviServer Run fucntion is not loop function. You should do game loop logic or sleep the main thread. The DokeviServer provides network handler logic, but it is can't helps game logic to process. 

### Step 3 : How to work iocp thread in dokevi-server?

 The dokevi-server use io completion port inside. The dokevi-server designed proact type frameworks. The server use windows network functions as AcceptEx(), WSASend(), WSARecv() ... 

 And then, When client requests to access server, the server call the accept handler and allocate client of the right type by ports. Also, When server send packet to client, client allocate packet from atomic-packet pool. The server reuses sockets and packets and uses minimal lock in all parts of the packet sending and receiving. 

 It is so powerful.

### Step 4 : Write log using LogManager(spdlog) easier

 The dokevi-server uses spdlog library. The spdlog very easy to write log messages to cmd window and text files. The dokevi-server include spdlog warpping class called LogManager. It easies to use.

> LOG_TRACE("text %d ...", &param1, &param2 ... &param3);
> Function Types : LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_CRITICAL

 You can use any types do you want. But LOG_CRITICAL makes force crash the program. Use the carefully.
  
## Documents

 If you want more information about The Dokevi-Network, you should click at the below link.

> https://github.com/kaygundraka/Dokevi-Network/wiki/
  
## Release History

* v1.0.0
    * Alpha: base framework update

## Maker

KaygunDraka – 69277660@naver.com (you must write '[Dokevi-Network]' at email title)

Distributed under the MIT license.
