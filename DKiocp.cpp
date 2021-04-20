#include "DKiocp.h"

#include <boost/thread.hpp>
#include <loki\Function.h>

#include <DKobject.h>


static int BUFFER_SIZE = 4096;
static volatile LONG g_nOutstandingSends = 0;

DKiocp::DKiocp(void)
{
	::GetSystemInfo(&m_sysinfo);
	m_nWaitCount = min(MAX_COMPLETION_THREAD_COUNT, m_sysinfo.dwNumberOfProcessors);

	// Round the buffer size to the next increment of the page size
    if ((BUFFER_SIZE % m_sysinfo.dwPageSize) != 0)
    {
        BUFFER_SIZE = ((BUFFER_SIZE / m_sysinfo.dwPageSize) + 1) * m_sysinfo.dwPageSize;
    }
}


DKiocp::~DKiocp(void)
{
    WSACleanup();
}


void DKiocp::EnqueuePendingOperation(BUFFER_OBJ *obj, int op)
{
	SCOPE_LOCK(m_syncPendingSend);

    if (op == OP_READ)
        ;
    else if (op == OP_WRITE)
        InterlockedIncrement(&obj->sock->PendingSend);

	m_listPendingSend.push_back(obj);
    return;
}

//
// Function: DequeuePendingOperation
//
// Description:
//    Dequeues the first entry in the list.
//
BUFFER_OBJ * DKiocp::DequeuePendingOperation(int op)
{
    BUFFER_OBJ *obj=NULL;
	SCOPE_LOCK(m_syncPendingSend);
	if(!m_listPendingSend.empty())
	{
		BUFFER_OBJ* obj = *m_listPendingSend.begin();
		m_listPendingSend.pop_front();

		if (op == OP_WRITE)
            InterlockedDecrement(&obj->sock->PendingSend);
	}
    return obj;
}

//
// Function: ProcessPendingOperations
//
// Description:
//    This function goes through the list of pending send operations and posts them
//    as long as the maximum number of ouststanding sends is not exceeded.
//
void DKiocp::ProcessPendingOperations()
{
    BUFFER_OBJ *sendobj=NULL;

    while(g_nOutstandingSends < MAX_OVERLAPPED_SENDS)
    {
        sendobj = DequeuePendingOperation(OP_WRITE);
        if (sendobj)
        {
            if (PostSend(sendobj->sock, sendobj) == SOCKET_ERROR)
            {
                // Cleanup
                printf("ProcessPendingOperations: PostSend failed!\n");

                FreeBufferObj(sendobj);

                break;
            }
        }
        else
        {
            break;
        }
    }

    return;
}

//
// Function: InsertPendingAccept
//
// Description:
//    Inserts a pending accept operation into the listening object.
//
void DKiocp::InsertPendingAccept(LISTEN_OBJ *listenobj, BUFFER_OBJ *obj)
{
	SCOPE_LOCK(listenobj->ListenCritSec);
    EnterCriticalSection(&listenobj->ListenCritSec);
	////////////////////////////////////////////////////
	listenobj->PendingAccepts.push_front(obj);
    ////////////////////////////////////////////////////
	LeaveCriticalSection(&listenobj->ListenCritSec);
}

//
// Function: RemovePendingAccept
//
// Description:
//    Removes the indicated accept buffer object from the list of pending
//    accepts in the listening object.
//
void DKiocp::RemovePendingAccept(LISTEN_OBJ *listenobj, BUFFER_OBJ *obj)
{
    EnterCriticalSection(&listenobj->ListenCritSec);
	////////////////////////////////////////////////////
	auto pos = std::find(listenobj->PendingAccepts.begin(), listenobj->PendingAccepts.end(), obj);
	if(pos != listenobj->PendingAccepts.end())
		listenobj->PendingAccepts.erase(pos);
	////////////////////////////////////////////////////
	LeaveCriticalSection(&listenobj->ListenCritSec);
}

//
// Function: GetBufferObj
// 
// Description:
//    Allocate a BUFFER_OBJ. A lookaside list is maintained to increase performance
//    as these objects are allocated frequently.
//
BUFFER_OBJ *DKiocp::GetBufferObj(int buflen)
{
    BUFFER_OBJ *newobj=NULL;

    EnterCriticalSection(&m_syncBufferList);
	if(m_listFreeBuffer.empty())
    {
        // Allocate the object
        newobj = (BUFFER_OBJ *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BUFFER_OBJ) + (sizeof(BYTE) * buflen));
        if (newobj == NULL)
        {
            //fprintf(stderr, "GetBufferObj: HeapAlloc failed: %d\n", GetLastError());
			exit(1);
        }
    }
    else
    {
		newobj = *m_listFreeBuffer.begin();
		m_listFreeBuffer.pop_front();
    }
    LeaveCriticalSection(&m_syncBufferList);
    
    if (newobj)
    {
        newobj->buf     = (char *)(((char *)newobj) + sizeof(BUFFER_OBJ));
        newobj->buflen  = buflen;
        newobj->addrlen = sizeof(newobj->addr);
    }

    return newobj;
}

//
// Function: FreeBufferObj
// 
// Description:
//    Free the buffer object. This adds the object to the free lookaside list.
//
void DKiocp::FreeBufferObj(BUFFER_OBJ *obj)
{
    EnterCriticalSection(&m_syncBufferList);
    
	memset(obj, 0, sizeof(BUFFER_OBJ) + BUFFER_SIZE);
	m_listFreeBuffer.push_back(obj);

    LeaveCriticalSection(&m_syncBufferList);
}

//
// Function: GetClienctContext
//
// Description:
//    Allocate a socket object and initialize its members. A socket object is
//    allocated for each socket created (either by socket or accept).
//    Socket objects are returned from a lookaside list if available. Otherwise,
//    a new object is allocated.
//
CLIENT_CONTEXT* DKiocp::GetClienctContext(SOCKET s, int af)
{
    CLIENT_CONTEXT  *pContext=NULL;

    EnterCriticalSection(&m_syncSocketList);
	if(m_listFreeClientContext.empty())
    {
        pContext = (CLIENT_CONTEXT *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CLIENT_CONTEXT));
        if (pContext == NULL)
        {
            //fprintf(stderr, "GetClienctContext: HeapAlloc failed: %d\n", GetLastError());
        }
        else
        {
            InitializeCriticalSection(&pContext->SockCritSec);
        }
    }
    else
    {
		pContext = *m_listFreeClientContext.begin();
		m_listFreeClientContext.pop_front();
    }
    LeaveCriticalSection(&m_syncSocketList);

    // Initialize the members
    if (pContext)
    {
        pContext->s  = s;
        pContext->af = af;
    }

    return pContext;
}

//
// Function: FreeClientContext
//
// Description:
//    Frees a socket object. The object is added to the lookaside list.
//
void DKiocp::FreeClientContext(CLIENT_CONTEXT *obj)
{
    CRITICAL_SECTION cstmp;
    BUFFER_OBJ      *ptr=NULL;

    // Close the socket if it hasn't already been closed
    if (obj->s != INVALID_SOCKET)
    {
        printf("FreeClientContext: closing socket\n");
        closesocket(obj->s);
        obj->s = INVALID_SOCKET;
    }

    EnterCriticalSection(&m_syncSocketList);

    cstmp = obj->SockCritSec;
    memset(obj, 0, sizeof(CLIENT_CONTEXT));
    obj->SockCritSec = cstmp;

	m_listFreeClientContext.push_back(obj);

    LeaveCriticalSection(&m_syncSocketList);
}


//
// Function: PrintStatistics
//
// Description:
//    Print the send/recv statistics for the server
//
void DKiocp::PrintStatistics()
{
    
}

//
// Function: PostRecv
// 
// Description: 
//    Post an overlapped receive operation on the socket.
//
int DKiocp::PostRecv(CLIENT_CONTEXT *sock, BUFFER_OBJ *recvobj)
{
    WSABUF  wbuf;
    DWORD   bytes,
            flags;
    int     rc;

    recvobj->operation = OP_READ;

    wbuf.buf = recvobj->buf;
    wbuf.len = recvobj->buflen;

    flags = 0;

    EnterCriticalSection(&sock->SockCritSec);
	
	////////////////////////////////////////////////////////////////////
    rc = WSARecv(sock->s, &wbuf, 1, &bytes, &flags, &recvobj->ol, NULL);
	////////////////////////////////////////////////////////////////////

    if (rc == SOCKET_ERROR)
    {
        rc = NO_ERROR;
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            //dbgprint("PostRecv: WSARecv* failed: %d\n", WSAGetLastError());
            rc = SOCKET_ERROR;
        }
    }
    if (rc == NO_ERROR)
    {
		printf("recv(%d)  (%d)bytes\n", sock->s, bytes);
        // Increment outstanding overlapped operations
        InterlockedIncrement(&sock->OutstandingRecv);
    }

    LeaveCriticalSection(&sock->SockCritSec);

    return rc;
}

//
// Function: PostSend
// 
// Description:
//    Post an overlapped send operation on the socket.
//
int DKiocp::PostSend(CLIENT_CONTEXT *sock, BUFFER_OBJ *sendobj)
{
    WSABUF  wbuf;
    DWORD   bytes;
    int     rc, err;

    sendobj->operation = OP_WRITE;

    wbuf.buf = sendobj->buf;
    wbuf.len = sendobj->buflen;

    EnterCriticalSection(&sock->SockCritSec);

	////////////////////////////////////////////////////////////////////
    rc = WSASend(sock->s, &wbuf, 1, &bytes, 0, &sendobj->ol, NULL);
	////////////////////////////////////////////////////////////////////

    if (rc == SOCKET_ERROR)
    {
        rc = NO_ERROR;
        if ((err = WSAGetLastError()) != WSA_IO_PENDING)
        {
            if (err == WSAENOBUFS)
                DebugBreak();

            //dbgprint("PostSend: WSASend* failed: %d [internal = %d]\n", WSAGetLastError(), sendobj->ol.Internal);
            rc = SOCKET_ERROR;
        }
    }
    if (rc == NO_ERROR)
    {
        // Increment the outstanding operation count
        InterlockedIncrement(&sock->OutstandingSend);

        InterlockedIncrement(&g_nOutstandingSends);
    }

    LeaveCriticalSection(&sock->SockCritSec);

    return rc;
}

//
// Function: PostAccept
// 
// Description:
//    Post an overlapped accept on a listening socket.
//
int DKiocp::PostAccept(LISTEN_OBJ *listen, BUFFER_OBJ *acceptobj)
{
    DWORD   bytes;
    int     rc;

    acceptobj->operation = OP_ACCEPT;

    // Create the client socket for an incoming connection
    acceptobj->sclient = socket(listen->AddressFamily, SOCK_STREAM, IPPROTO_TCP);
    if (acceptobj->sclient == INVALID_SOCKET)
    {
        //fprintf(stderr, "PostAccept: socket failed: %d\n", WSAGetLastError());
        return -1;
    }

    rc = listen->lpfnAcceptEx(
            listen->s,
            acceptobj->sclient,
            acceptobj->buf,
            acceptobj->buflen - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
            sizeof(SOCKADDR_STORAGE) + 16,
            sizeof(SOCKADDR_STORAGE) + 16,
           &bytes,
           &acceptobj->ol
            );
    if (rc == FALSE)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            printf("PostAccept: AcceptEx failed: %d\n",
                    WSAGetLastError());
            return SOCKET_ERROR;
        }
    }

    // Increment the outstanding overlapped count for this socket
    InterlockedIncrement(&listen->PendingAcceptCount);

    return NO_ERROR;
}

//
// Function: HandleIo
//
// Description:
//    This function handles the IO on a socket. In the event of a receive, the 
//    completed receive is posted again. For completed accepts, another AcceptEx
//    is posted. For completed sends, the buffer is freed.
//
void DKiocp::HandleIo(ULONG_PTR key, BUFFER_OBJ *buf, DWORD BytesTransfered, DWORD error)
{
    LISTEN_OBJ *listenobj=NULL;
    CLIENT_CONTEXT *pContext=NULL,
               *clientobj=NULL;     // New client object for accepted connections
    BUFFER_OBJ *recvobj=NULL,       // Used to post new receives on accepted connections
               *sendobj=NULL;       // Used to post new sends for data received
    BOOL        bCleanupSocket;

    if (error != 0)
    {
        //dbgprint("OP = %d; Error = %d\n", buf->operation, error);
    }

    bCleanupSocket = FALSE;

    if (error != NO_ERROR)
    {
        // An error occured on a TCP socket, free the associated per I/O buffer
        // and see if there are any more outstanding operations. If so we must
        // wait until they are complete as well.
        //
        if (buf->operation != OP_ACCEPT)
        {
            pContext = (CLIENT_CONTEXT *)key;
            if (buf->operation == OP_READ)
            {
                if ((InterlockedDecrement(&pContext->OutstandingRecv) == 0) &&
                    (pContext->OutstandingSend == 0) )
                {
                    //dbgprint("Freeing socket obj in GetOverlappedResult\n");
                    FreeClientContext(pContext);
                }
            }
            else if (buf->operation == OP_WRITE)
            {
                if ((InterlockedDecrement(&pContext->OutstandingSend) == 0) &&
                    (pContext->OutstandingRecv == 0) )
                {
                    //dbgprint("Freeing socket obj in GetOverlappedResult\n");
                    FreeClientContext(pContext);
                }
            }
        }
        else
        {
            listenobj = (LISTEN_OBJ *)key;

            printf("Accept failed\n");

            closesocket(buf->sclient);
            buf->sclient = INVALID_SOCKET;
        }

        FreeBufferObj(buf);

        return;
    }

    if (buf->operation == OP_ACCEPT)
    {
		///
		///
		/// 새로운 클라이언트가 접속하면 이쪽으로 오게된다.
		///
		///

        HANDLE            hrc;
        SOCKADDR_STORAGE *LocalSockaddr=NULL,
                         *RemoteSockaddr=NULL;
        int               LocalSockaddrLen,
                          RemoteSockaddrLen;

        listenobj = (LISTEN_OBJ *)key;

        // Update counters
        InterlockedIncrement(&gConnections);
        InterlockedIncrement(&gConnectionsLast);
        InterlockedDecrement(&listenobj->PendingAcceptCount);
        InterlockedExchangeAdd(&gBytesRead, BytesTransfered);
        InterlockedExchangeAdd(&gBytesReadLast, BytesTransfered);

        // Print the client's addresss
        listenobj->lpfnGetAcceptExSockaddrs(
                buf->buf,
                buf->buflen - ((sizeof(SOCKADDR_STORAGE) + 16) * 2),
                sizeof(SOCKADDR_STORAGE) + 16,
                sizeof(SOCKADDR_STORAGE) + 16,
                (SOCKADDR **)&LocalSockaddr,
               &LocalSockaddrLen,
                (SOCKADDR **)&RemoteSockaddr,
               &RemoteSockaddrLen
                );

        RemovePendingAccept(listenobj, buf);

        // Get a new CLIENT_CONTEXT for the client connection
        clientobj = GetClienctContext(buf->sclient, listenobj->AddressFamily);
        if (clientobj)
        {
            // Associate the new connection to our completion port

			///
			///////////////////////////////////////////////////////////////////////////////////
            hrc = CreateIoCompletionPort((HANDLE)clientobj->s, m_hMainCompPort, (ULONG_PTR)clientobj, 0);
			///////////////////////////////////////////////////////////////////////////////////
			///

            if (hrc == NULL)
            {
                //fprintf(stderr, "CompletionThread: CreateIoCompletionPort failed: %d\n",
                        GetLastError());
                return;
            }

            sendobj = buf;
            sendobj->buflen = BytesTransfered;

            // Post the send - this is the first one for this connection so just do it
            sendobj->sock = clientobj;
            //PostSend(clientobj, sendobj);
            EnqueuePendingOperation(sendobj, OP_WRITE);
        }
        else
        {
            // Can't allocate a socket structure so close the connection
            closesocket(buf->sclient);
            buf->sclient = INVALID_SOCKET;
            FreeBufferObj(buf);
        }
        
		if (error != NO_ERROR)
		{
            // Check for socket closure
            EnterCriticalSection(&clientobj->SockCritSec);
            if ( (clientobj->OutstandingSend == 0) &&
                 (clientobj->OutstandingRecv == 0) )
            {
                closesocket(clientobj->s);
                clientobj->s = INVALID_SOCKET;
                FreeClientContext(clientobj);
            }
            else
            {
                clientobj->bClosing = TRUE;
            }
            LeaveCriticalSection(&clientobj->SockCritSec);

            error = NO_ERROR;
		}

        InterlockedIncrement(&listenobj->RepostCount);
        SetEvent(listenobj->RepostAccept);
    }
    else if (buf->operation == OP_READ)
    {
        pContext = (CLIENT_CONTEXT *)key;

        InterlockedDecrement(&pContext->OutstandingRecv);

        //
        // Receive completed successfully
        //
        if (BytesTransfered > 0)
        {
            InterlockedExchangeAdd(&gBytesRead, BytesTransfered);
            InterlockedExchangeAdd(&gBytesReadLast, BytesTransfered);

            // Make the recv a send
            sendobj         = buf;
            sendobj->buflen = BytesTransfered;

            sendobj->sock = pContext;
            //PostSend(pContext, sendobj);
            EnqueuePendingOperation(sendobj, OP_WRITE);
        }
        else
        {
            ////dbgprint("Got 0 byte receive\n");

            // Graceful close - the receive returned 0 bytes read
            pContext->bClosing = TRUE;

            // Free the receive buffer
            FreeBufferObj(buf);

            // If this was the last outstanding operation on socket, clean it up
            EnterCriticalSection(&pContext->SockCritSec);

            if ((pContext->OutstandingSend == 0) && 
                (pContext->OutstandingRecv == 0) )
            {
                bCleanupSocket = TRUE;
            }
            LeaveCriticalSection(&pContext->SockCritSec);
        }
    }
    else if (buf->operation == OP_WRITE)
    {
        pContext = (CLIENT_CONTEXT *)key;

        InterlockedDecrement(&pContext->OutstandingSend);
        InterlockedDecrement(&g_nOutstandingSends);

        buf->buflen = BUFFER_SIZE;

        if (pContext->bClosing == FALSE)
        {
            buf->sock = pContext;
            PostRecv(pContext, buf);
        }
    }

    ProcessPendingOperations();

    if (pContext)
    {
        if (error != NO_ERROR)
        {
            printf("err = %d\n", error);
            pContext->bClosing = TRUE;
        }

        //
        // Check to see if socket is closing
        //
        if ( (pContext->OutstandingSend == 0) &&
             (pContext->OutstandingRecv == 0) &&
             (pContext->bClosing) )
        {
            bCleanupSocket = TRUE;
        }

        if (bCleanupSocket)
        {
            closesocket(pContext->s);
            pContext->s = INVALID_SOCKET;

            FreeClientContext(pContext);
        }
    }

    return;
}

//
// Function: CompletionThread
// 
// Description:
//    This is the completion thread which services our completion port. One of
//    these threads is created per processor on the system. The thread sits in 
//    an infinite loop calling GetQueuedCompletionStatus and handling socket
//    IO that completed.
//
void DKiocp::CompletionThread()
{
    ULONG_PTR    Key;
    SOCKET       s;
    BUFFER_OBJ  *bufobj=NULL;           // Per I/O object for completed I/O
    OVERLAPPED  *lpOverlapped=NULL;     // Pointer to overlapped structure for completed I/O
    DWORD        BytesTransfered,       // Number of bytes transfered
                 Flags;                 // Flags for completed I/O
    int          rc, 
                 error;

    while (1)
    {
        error = NO_ERROR;

        rc = ::GetQueuedCompletionStatus(m_hMainCompPort, &BytesTransfered, (PULONG_PTR)&Key, &lpOverlapped, INFINITE);

        bufobj = CONTAINING_RECORD(lpOverlapped, BUFFER_OBJ, ol);

        if (rc == FALSE)
        {
            // If the call fails, call WSAGetOverlappedResult to translate the
            //    error code into a Winsock error code.
            if (bufobj->operation == OP_ACCEPT)
            {
                s = ((LISTEN_OBJ *)Key)->s;
            }
            else
            {
                s = ((CLIENT_CONTEXT *)Key)->s;
            }
          
            //dbgprint("CompletionThread: GetQueuedCompletionStatus failed: %d [0x%x]\n", GetLastError(), lpOverlapped->Internal);

            rc = ::WSAGetOverlappedResult(s, &bufobj->ol, &BytesTransfered, FALSE, &Flags);

            if (rc == FALSE)
            {
                error = WSAGetLastError();
            }
        }

        // Handle the IO operation
        HandleIo(Key, bufobj, BytesTransfered, error);
    }
}


BOOL DKiocp::Init()
{
    WSADATA          wsd;
    if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
    {
        //fprintf(stderr, "unable to load Winsock!\n");
        return -1;
    }

    InitializeCriticalSection(&m_syncSocketList);
    InitializeCriticalSection(&m_syncBufferList);
    InitializeCriticalSection(&m_syncPendingSend);

    // Create the completion port used by this server
    m_hMainCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
    if (m_hMainCompPort == NULL)
    {
        //fprintf(stderr, "CreateIoCompletionPort failed: %d\n", GetLastError());
        return -1;
    }


	return TRUE;
}

//
// Function: main
//
// Description:
//      This is the main program. It parses the command line and creates
//      the main socket. For TCP the socket is used to accept incoming 
//      client connections. Each client TCP connection is handed off to 
//      a worker thread which will receive any data on that connection 
//      until the connection is closed.
//


BOOL DKiocp::Start()
{
    if(!Init())
		return FALSE;

    GUID             guidAcceptEx = WSAID_ACCEPTEX,
                     guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
    DWORD            bytes;

    struct addrinfo *res=NULL,
                    *ptr=NULL;

    // Create the worker threads to service the completion notifications
    for(UINT i = 0 ; i < m_nWaitCount ; ++i)
    {
		Loki::Functor<void>  fnc(this, &DKiocp::CompletionThread);
		boost::thread thrd(fnc);

        m_hWait[i] = (HANDLE)thrd.native_handle();
        if(m_hWait[i] == NULL)
        {
            ////fprintf(stderr, "CreatThread failed: %d\n", GetLastError());
            return -1;
        }
    }

    // Obtain the "wildcard" addresses for all the available address families
    res = ResolveAddress("", "999", AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (res == NULL)
    {
        //fprintf(stderr, "ResolveAddress failed to return any addresses!\n");
        return -1;
    }

    // For each local address returned, create a listening/receiving socket
    ptr = res;
    while (ptr)
    {
        LISTEN_OBJ* listenobj = new LISTEN_OBJ;

        if (listenobj == NULL)
        {
            //fprintf(stderr, "Out of memory!\n");
            return -1;
        }

        InitializeCriticalSection(&listenobj->ListenCritSec);
        
        // Save off the address family of this socket
        listenobj->AddressFamily = ptr->ai_family;

        // create the socket
        listenobj->s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (listenobj->s == INVALID_SOCKET)
        {
            //fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            return -1;
        }

        // Create an event to register for FD_ACCEPT events on
        listenobj->AcceptEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (listenobj->AcceptEvent == NULL)
        {
            //fprintf(stderr, "CreateEvent failed: %d\n", GetLastError());
            return -1;
        }

        listenobj->RepostAccept = ::CreateEvent(NULL, TRUE, FALSE, NULL);
        if (listenobj->RepostAccept == NULL)
        {
            //fprintf(stderr, "CreateSemaphore failed: %d\n", GetLastError());
            return -1;
        }

        // Add the event to the liste of waiting events
        m_hWait[m_nWaitCount++] = listenobj->AcceptEvent;

        m_hWait[m_nWaitCount++] = listenobj->RepostAccept;

        // Associate the socket and its CLIENT_CONTEXT to the completion port
        HANDLE hrc = CreateIoCompletionPort((HANDLE)listenobj->s, m_hMainCompPort, (ULONG_PTR)listenobj, 0);
        if (hrc == NULL)
        {
            //fprintf(stderr, "CreateIoCompletionPort failed: %d\n", GetLastError());
            return -1;
        }

        // bind the socket to a local address and port
        int rc = ::bind(listenobj->s, ptr->ai_addr, ptr->ai_addrlen);
        if (rc == SOCKET_ERROR)
        {
            //fprintf(stderr, "bind failed: %d\n", WSAGetLastError());
            return -1;
        }

        // Need to load the Winsock extension functions from each provider
        //    -- e.g. AF_INET and AF_INET6. 
        rc = WSAIoctl(
                listenobj->s,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
               &guidAcceptEx,
                sizeof(guidAcceptEx),
               &listenobj->lpfnAcceptEx,
                sizeof(listenobj->lpfnAcceptEx),
               &bytes,
                NULL,
                NULL
                );
        if (rc == SOCKET_ERROR)
        {
            //fprintf(stderr, "WSAIoctl: SIO_GET_EXTENSION_FUNCTION_POINTER failed: %d\n",
                    WSAGetLastError());
            return -1;
        }

        // Load the Winsock extensions from each provider
        rc = WSAIoctl(
                listenobj->s,
                SIO_GET_EXTENSION_FUNCTION_POINTER,
               &guidGetAcceptExSockaddrs,
                sizeof(guidGetAcceptExSockaddrs),
               &listenobj->lpfnGetAcceptExSockaddrs,
                sizeof(listenobj->lpfnGetAcceptExSockaddrs),
               &bytes,
                NULL,
                NULL
                );
        if (rc == SOCKET_ERROR)
        {
            //fprintf(stderr, "WSAIoctl: SIO_GET_EXTENSION_FUNCTION_POINTER faled: %d\n",
                    WSAGetLastError());
            return -1;
        }

        // Put the socket into listening mode
        rc = listen(listenobj->s, 200);
        if (rc == SOCKET_ERROR)
        {
            //fprintf(stderr, "listen failed: %d\n", WSAGetLastError());
            return -1;
        }

        // Register for FD_ACCEPT notification on listening socket
        rc = WSAEventSelect(listenobj->s, listenobj->AcceptEvent, FD_ACCEPT);
        if (rc == SOCKET_ERROR)
        {
            //fprintf(stderr, "WSAEventSelect failed: %d\n", WSAGetLastError());
            return -1;
        }

        // Initiate the initial accepts for each listen socket
        for(UINT i = 0; i < DEFAULT_OVERLAPPED_COUNT ; i++)
        {
			BUFFER_OBJ* acceptobj = GetBufferObj(BUFFER_SIZE);
            if (acceptobj == NULL)
            {
                //fprintf(stderr, "Out of memory!\n");
                return -1;
            }

            acceptobj->PostAccept = listenobj->AcceptEvent;

            InsertPendingAccept(listenobj, acceptobj);

            PostAccept(listenobj, acceptobj);
        }

        // Maintain a list of the listening socket structures
		m_listListen.push_back(listenobj);
        
        ptr = ptr->ai_next;
    }
    // free the addrinfo structure for the 'bind' address
    freeaddrinfo(res);

	Loki::Functor<void>  fnc(this, &DKiocp::MonitorThread);
	boost::thread thrd(fnc);
    m_hMonitorThread = (HANDLE)thrd.native_handle();
    return 0;
}

void DKiocp::MonitorThread()
{
	int interval = 0;
	while (1)
    {
        DWORD rc = WSAWaitForMultipleEvents(m_nWaitCount, m_hWait, FALSE, 5000, FALSE);


        if (rc == WAIT_FAILED)
        {
            //fprintf(stderr, "WSAWaitForMultipleEvents failed: %d\n", WSAGetLastError());
            break;
        }
        else if (rc == WAIT_TIMEOUT)
        {
            interval++;

            PrintStatistics();

            if (interval == 36)
            {
                int          optval,
                             optlen;

                // For TCP, cycle through all the outstanding AcceptEx operations
                //   to see if any of the client sockets have been connected but
                //   haven't received any data. If so, close them as they could be
                //   a denial of service attack.

				for(auto pos = m_listListen.begin() ; pos != m_listListen.end() ; ++pos)
				{
					LISTEN_OBJ* listenobj = *pos;
                
                    EnterCriticalSection(&listenobj->ListenCritSec);

					for(auto apos = listenobj->PendingAccepts.begin() ; apos != listenobj->PendingAccepts.end() ; ++apos)
					{
						BUFFER_OBJ* acceptobj = *apos;

                        optlen = sizeof(optval);
                        rc = getsockopt(
                                acceptobj->sclient,
                                SOL_SOCKET,
                                SO_CONNECT_TIME,
                                (char *)&optval,
                               &optlen
                                );
                        if (rc == SOCKET_ERROR)
                        {
                            //fprintf(stderr, "getsockopt: SO_CONNECT_TIME failed: %d\n", WSAGetLastError());
                        }
                        else
                        {
                            // If the socket has been connected for more than 5 minutes,
                            //    close it. If closed, the AcceptEx call will fail in the
                            //    completion thread.
                            if ((optval != 0xFFFFFFFF) && (optval > 300))
                            {
                                printf("closing stale handle\n");
                                closesocket(acceptobj->sclient);
                                acceptobj->sclient = INVALID_SOCKET;
                            }
                        }
                    }
                    LeaveCriticalSection(&listenobj->ListenCritSec);
                }
                interval = 0;
            }
        }
        else
        {
            int index;

            index = rc - WAIT_OBJECT_0;

            for( ; index < m_nWaitCount ; index++)
            {
                rc = WaitForSingleObject(m_hWait[index], 0);
                if (rc == WAIT_FAILED || rc == WAIT_TIMEOUT)
                {
                    continue;
                }
                if (index < (int)m_sysinfo.dwNumberOfProcessors)
                {
                    // One of the completion threads exited
                    //   This is bad so just bail - a real server would want
                    //   to gracefully exit but this is just a sample ...
                    ExitProcess(-1);
                }
                else
                {
                    // An FD_ACCEPT event occured
					LISTEN_OBJ* listenobj = NULL;
					for(auto pos = m_listListen.begin() ; pos != m_listListen.end() ; ++pos)
					{
						listenobj = *pos;
                    
                        if ((listenobj->AcceptEvent == m_hWait[index]) || 
                                (listenobj->RepostAccept  == m_hWait[index]))
                            break;
                    }

                    if (listenobj)
                    {
                        WSANETWORKEVENTS ne;
                        int              limit=0;

                        if (listenobj->AcceptEvent == m_hWait[index])
                        {
                            // EnumNetworkEvents to see if FD_ACCEPT was set
                            rc = WSAEnumNetworkEvents(
                                    listenobj->s,
                                    listenobj->AcceptEvent,
                                    &ne
                                                     );
                            if (rc == SOCKET_ERROR)
                            {
                                //fprintf(stderr, "WSAEnumNetworkEvents failed: %d\n",
                                        WSAGetLastError());
                            }
                            if ((ne.lNetworkEvents & FD_ACCEPT) == FD_ACCEPT)
                            {
                                // We got an FD_ACCEPT so post multiple accepts to 
                                // cover the burst
                                limit = BURST_ACCEPT_COUNT;
                            }
                        }
                        else if (listenobj->RepostAccept == m_hWait[index])
                        {
                            // Semaphore is signaled
                            limit = InterlockedExchange(&listenobj->RepostCount, 0);

                            ResetEvent(listenobj->RepostAccept);
                        }

                        i = 0;
                        while ( (i++ < limit) &&
                                (listenobj->PendingAcceptCount < MAX_OVERLAPPED_ACCEPTS) )
                        {
                            BUFFER_OBJ* acceptobj = GetBufferObj(BUFFER_SIZE);
                            if (acceptobj)
                            {
                                acceptobj->PostAccept = listenobj->AcceptEvent;

                                InsertPendingAccept(listenobj, acceptobj);

                                PostAccept(listenobj, acceptobj);
                            }
                        }
                    }
                }
            }
        }
    }
}