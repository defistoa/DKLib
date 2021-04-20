#pragma once


#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
using namespace std;
#include "resolve.h"
#pragma comment(lib,"ws2_32.lib")

#include <DKcommon.h>


#define DEFAULT_OVERLAPPED_COUNT    5      // Number of overlapped recv per socket
#define MAX_OVERLAPPED_ACCEPTS      500
#define MAX_OVERLAPPED_SENDS        200
#define MAX_OVERLAPPED_RECVS        200
#define MAX_COMPLETION_THREAD_COUNT 32     // Maximum number of completion threads allowed
#define BURST_ACCEPT_COUNT          100

int BUFFER_SIZE    = 4096,
    gMaxSends      = MAX_OVERLAPPED_SENDS;

enum OPERATION
{
	OP_NONE = -1,
	OP_ACCEPT =      0,                   // AcceptEx
	OP_READ   =      1,                   // WSARecv/WSARecvFrom
	OP_WRITE  =      2,                   // WSASend/WSASendTo
};

struct BUFFER_OBJ
{
    WSAOVERLAPPED        ol;

    SOCKET               sclient;       // Used for AcceptEx client socket
    HANDLE               PostAccept;

    char                *buf;           // Buffer for recv/send/AcceptEx
    int                  buflen;        // Length of the buffer

    OPERATION           operation;     // Type of operation issued
    SOCKADDR_STORAGE     addr;
    int                  addrlen;

    struct CLIENT_CONTEXT  *sock;
};

//
// This is our per socket buffer. It contains information about the socket handle
//    which is returned from each GetQueuedCompletionStatus call.
//
struct CLIENT_CONTEXT
{
    SOCKET             s;               // Socket handle

    int                af,              // Address family of socket (AF_INET, AF_INET6)
                       bClosing;        // Is the socket closing?

    volatile LONG      OutstandingRecv, // Number of outstanding overlapped ops on 
                       OutstandingSend,
                       PendingSend;

	CRITICAL_SECTION   SockCritSec;     // Protect access to this structure
};


typedef struct _LISTEN_OBJ
{
    SOCKET          s;

    int             AddressFamily;

    list<BUFFER_OBJ*>	PendingAccepts; // Pending AcceptEx buffers 
                               
    volatile long   PendingAcceptCount;

    HANDLE          AcceptEvent;
    HANDLE          RepostAccept;
    volatile long   RepostCount;

    // Pointers to Microsoft specific extensions.
    LPFN_ACCEPTEX             lpfnAcceptEx;
    LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;

    CRITICAL_SECTION ListenCritSec;
	
	_LISTEN_OBJ()
	{
		s = NULL;
		AddressFamily = 0;
		PendingAcceptCount = 0;
		AcceptEvent = NULL;
		RepostAccept = NULL;
		RepostCount = 0;
		lpfnAcceptEx = NULL;
		lpfnGetAcceptExSockaddrs = NULL;
		ZeroMemory(&ListenCritSec, sizeof(ListenCritSec));
	}

} LISTEN_OBJ;

// Serialize access to the free lists below

// Lookaside lists for free buffers and socket objects


class DKiocp
{
public:
	DKiocp(void);
	virtual ~DKiocp(void);

	BOOL Start();

private:
	BOOL Init();

	void AppendLog(LPCTSTR szMsg, BOOL bError = FALSE);

	void EnqueuePendingOperation(BUFFER_OBJ *obj, int op);
	BUFFER_OBJ* DequeuePendingOperation(int op);
	void ProcessPendingOperations();
	void InsertPendingAccept(LISTEN_OBJ *listenobj, BUFFER_OBJ *obj);
	void RemovePendingAccept(LISTEN_OBJ *listenobj, BUFFER_OBJ *obj);
	
	BUFFER_OBJ* GetBufferObj(int buflen);
	void FreeBufferObj(BUFFER_OBJ *obj);
	
	CLIENT_CONTEXT* GetClienctContext(SOCKET s, int af);
	void FreeClientContext(CLIENT_CONTEXT *obj);

	void PrintStatistics();
	int PostRecv(CLIENT_CONTEXT *sock, BUFFER_OBJ *recvobj);
	int PostSend(CLIENT_CONTEXT *sock, BUFFER_OBJ *sendobj);
	int PostAccept(LISTEN_OBJ *listen, BUFFER_OBJ *acceptobj);
	void HandleIo(ULONG_PTR key, BUFFER_OBJ *buf, DWORD BytesTransfered, DWORD error);
	

	void CompletionThread();
	void MonitorThread();
private:
	HANDLE m_hMainCompPort;
	HANDLE m_hMonitorThread;
	HANDLE m_hWait[MAX_COMPLETION_THREAD_COUNT];
	UINT	m_nWaitCount;
	SYSTEM_INFO m_sysinfo;


	CRITICAL_SECTION m_syncBufferList, m_syncSocketList, m_syncPendingSend;

	list<LISTEN_OBJ*>	m_listListen;

	list<BUFFER_OBJ*> m_listFreeBuffer;
	list<CLIENT_CONTEXT*> m_listFreeClientContext;
	list<BUFFER_OBJ*> m_listPendingSend; //固贸府等 傈价
};

