#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <winsock2.h>
#include <WinSock2.h>

#include <Ws2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

typedef int socklen_t;

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
typedef int SOCKET;
const int NO_ERROR = 0;
const int INVALID_SOCKET = -1;
const int WSAECONNRESET = ECONNRESET;
const int WSAEWOULDBLOCK = EAGAIN;
const int SOCKET_ERROR = -1;
#endif

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <list>
#include <queue>
#include <deque>
#include <unordered_set>
#include <cassert>
#include <thread>
#include <bitset>
#include <set>
#include <mutex>
#include <iostream>
#include <atomic>
#include <array>
#include <shared_mutex>
#include <Dbghelp.h>

typedef int socklen_t;

#include "LogManager.h"
#include "StackTracer.h"
#include "LockStackTracer.h"

#pragma warning(disable:4996)

enum {
	MAX_THREAD = 128,
	MAX_CALL_STACK_DEPTH = 128,
	MAX_STACK_CHECK_THREAD_NUM = 1024,
	MAX_PACKET_BUF_SIZE = 4096,
};

#define SINGLETON(type) DokeviNet::type::GetInstance()

#define OUT /*OUT*/
#define IN /*IN*/

using namespace std;