#include "xweb.h"


static XWEB__clientInfo_t *XWEB__pClients = NULL;


XWEB__clientInfo_t * XWEB__HttpSvrGetClientsList(void)
{
    return (XWEB__pClients);
}

int XWEB__HttpParseUrl( char *url, char **hostname, char **port, char **path)
{
    int retCode = 0;

    char *p = NULL;
    char *protocol = NULL;

    /* Print the user url input */
    if (url)
    {
        printf("URL: %s\n", url);
    }
    else
    {
        fprintf(stderr, "No valid url\n");
        retCode = 1;
        goto labelExit;
    }
    
    /* Find the protocol */
    p = strstr(url, XWEB__PROTO_DELIMITER);
    if (p)
    {
        protocol = url;
        *p = XWEB__CHAR_NULL;
        p += strlen(XWEB__PROTO_DELIMITER);
    }
    else
    {
        p = url;
    }
    if (protocol)
    {
        if (strcmp(protocol, XWEB__PROTO_HTTP))
        {
            fprintf(stderr, "Unknown protocol '%s'. Only %s is supported.\n",
            protocol, XWEB__PROTO_HTTP);
            retCode = 2;
            goto labelExit;
        }
    }

    /* retreive the hostname */
    *hostname = p;
    while (*p && *p != ':' && *p != '/' && *p != '#') ++p;

    /* retrieve port */
    *port = "80";
    if (*p == ':')
    {
        *p++ = 0;
        *port = p;
    }
    while (*p && *p != '/' && *p != '#') ++p;


    /* retrieve path */
    *path = p;
    if (*p == '/')
    {
        *path = p + 1;
    }
    *p = 0;
    while (*p && *p != '#') ++p;
    {
        if (*p == '#')
        {
            *p = 0;
        }
    }

    /* show details */
    printf("hostname: %s\n", *hostname);
    printf("port: %s\n", *port);
    printf("path: %s\n", *path);
    retCode = 0;

labelExit:
    return (retCode);
}




int XWEB__HttpSendRequest( socket_t sfd, char *hostname, char *port, char *path)
{
    char buffer [512];
    int  retCode = 0;

    memset((void *)buffer, 0, 512);

    sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc xweb 1.0\r\n");
    sprintf(buffer + strlen(buffer), "\r\n");

    retCode = send(sfd, buffer, strlen(buffer), MSG_NOSIGNAL);
    if (retCode < 1)
    {
        fprintf(stderr, "send failed \n");
        retCode = 15;
        goto labelExit;
    }

    printf ("Sent Headers:\n%s", buffer);
    retCode = 0;
labelExit:
    return (retCode);
}




int XWEB__HttpConnectToHost( socket_t *sfd, char *hostname, char *port)
{
    char address_buffer[100];
    char service_buffer[100];
    struct addrinfo hints;
    struct addrinfo *peer_address;
    socket_t server;
    int    retCode = 0;

    memset((void *)&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    //hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    printf ("Configuring remote address...\n");
    if (getaddrinfo(hostname, port, &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo failed\n");
        retCode = 3;
        goto labelExit;
    }

    printf("Remote address is: ");
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);
    
    /* Create socket */
    printf("Creating socket...\n");
    *sfd = socket(peer_address->ai_family, 
                  peer_address->ai_socktype,
                  peer_address->ai_protocol);
    if (!(*sfd))
    {
        fprintf(stderr, "socket failed\n");
        retCode = 4;
        goto labelExit;
    }

    /* Connecting to server */
    if ( connect(*sfd, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "connect failed\n");
        retCode = 14;
        goto labelExit;
    }
    
    freeaddrinfo(peer_address);
    printf("Connected.\n\n");
    retCode = 0;

labelExit:
    return (retCode);
}




const char *XWEB__HttpGetContentType( const char *path)
{
    const char *lastDot = strrchr(path, '.');
    if (lastDot)
    {
        if (strcmp( lastDot, ".css") == 0) return "text/css";
        if (strcmp( lastDot, ".csv") == 0) return "text/csv";
        if (strcmp( lastDot, ".gif") == 0) return "image/gif";
        if (strcmp( lastDot, ".htm") == 0) return "text/html";
        if (strcmp( lastDot, ".html") == 0) return "text/html";
        if (strcmp( lastDot, ".ico") == 0) return "image/x-icon";
        if (strcmp( lastDot, ".jpeg") == 0) return "image/jpeg";
        if (strcmp( lastDot, ".jpg") == 0) return "image/jpeg";
        if (strcmp( lastDot, ".jpg") == 0) return "image/jpeg";
        if (strcmp( lastDot, ".js") == 0) return "application/javascript";
        if (strcmp( lastDot, ".json") == 0) return "application/json";
        if (strcmp( lastDot, ".png") == 0) return "image/png";
        if (strcmp( lastDot, ".pdf") == 0) return "application/pdf";
        if (strcmp( lastDot, ".svg") == 0) return "image/svg+xml";
        if (strcmp( lastDot, ".txt") == 0) return "txt/plain";
    }
    return "application/octet-stream";
}




int XWEB__HttpSvrCreateSock( socket_t *sfd, char *hostname, char *port)
{
    char address_buffer[100];
    char service_buffer[100];
    struct addrinfo hints;
    struct addrinfo *peer_address;
    socket_t server;
    int    retCode = 0;

    memset((void *)&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family   = AF_INET;
    //hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    printf ("Configuring remote address...\n");
    if (getaddrinfo(hostname, port, &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo failed\n");
        retCode = 3;
        goto labelExit;
    }
    
    /* Create socket */
    printf("Creating socket...\n");
    *sfd = socket(peer_address->ai_family, 
                  peer_address->ai_socktype,
                  peer_address->ai_protocol);
    if (!(*sfd))
    {
        fprintf(stderr, "socket failed\n");
        retCode = 4;
        goto labelExit;
    }

    /* Connecting to server */
    printf("Binding socket to local address ...\n");
    if ( bind(*sfd, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "bind failed\n");
        retCode = 14;
        goto labelExit;
    }
    
    freeaddrinfo(peer_address);
    printf("Listening.\n\n");
    if (listen(*sfd, 10) < 0)
    {
        fprintf(stderr, "listen failed\n");
        retCode = 20;
        goto labelExit;
    }
    retCode = 0;

labelExit:
    return (retCode);
}


int XWEB__HttpSvrGetClient( socket_t sfd, XWEB__clientInfo_t **pClientAddr)
{
    XWEB__clientInfo_t *pClient = XWEB__pClients;
    XWEB__clientInfo_t *pNewClient = NULL;
    int retCode = 0;

#if 0
    if (!pClientAddr)
    {
        fprintf(stderr, "Invalid client addr - nullptr \n");
        retCode = 21;
        goto labelExit;

    }
#endif
    /* Search for client in the client linked list */
    while( pClient )
    {
        if (pClient->socket == sfd)
        {
            break ;
        }
        pClient = pClient->pNext;
    }
    /* return if client is found. */
    if (pClient)
    {
        *pClientAddr = pClient;
        retCode = 0;
        goto labelExit;
    }

    /* create a new client if not found and add to client list */
    pNewClient = (XWEB__clientInfo_t *)calloc(1, sizeof(XWEB__clientInfo_t));
    if (!pNewClient)
    {
        fprintf(stderr, "out of memory\n");
        retCode = 22;
        goto labelExit;
    }
    memset((void *)pNewClient, 0, sizeof(XWEB__clientInfo_t));
    pNewClient->addrLen = sizeof(pNewClient->addr);
    pNewClient->pNext = XWEB__pClients;
    XWEB__pClients = pNewClient;
    *pClientAddr = pNewClient;

labelExit:
    return (retCode);
}



int  XWEB__HttpSvrDropClient( XWEB__clientInfo_t *pClient)
{
    int retCode = 0;
    XWEB__clientInfo_t **p = &XWEB__pClients;

    if (!pClient)
    {
        fprintf(stderr, "DropClient: Invalid client - nullptr \n");
        retCode = 21;
        goto labelExit;
    }

    close(pClient->socket);
    while (*p)
    {
        if (*p == pClient)
        {
            *p = pClient->pNext;
            free(pClient);
            retCode = 0;
            goto labelExit;
        }
        p = &(*p)->pNext;
    }

    fprintf(stderr, "DropClient: client not found.\n");
    retCode = 23;

labelExit:
    return (retCode);
}




const char *XWEB__HttpSvrGetClientAddr( XWEB__clientInfo_t *pClient)
{
    static char address_buffer[100];
    memset((void *)address_buffer, 0, 100);

    if (!pClient)
    {
        fprintf(stderr, "GetClientAddr: Invalid client - nullptr \n");
        goto labelExit;
    }
    getnameinfo( (struct sockaddr *)&(pClient->addr),
                 pClient->addrLen,
                 address_buffer, sizeof(address_buffer),
                 0,0,  /* ports buffer and buffer len */
                 NI_NUMERICHOST);
labelExit:
    return (address_buffer);
}




fd_set XWEB__HttpSvrWaitOnClients( socket_t srv)
{
    fd_set              reads;
    socket_t            maxSock = srv;
    XWEB__clientInfo_t *pClient = XWEB__pClients;

    FD_ZERO(&reads);
    FD_SET(srv, &reads);

    while( pClient )
    {
        FD_SET( pClient->socket, &reads);
        if ( pClient->socket > maxSock )
        {
            maxSock = pClient->socket;
        }
        pClient = pClient->pNext;
    }

    if ( select(maxSock+1, &reads, 0, 0, 0) < 0)
    {
        fprintf(stderr, "WaitOnClients: select failed \n");
        exit(1);
    }
labelExit:
    return (reads);
}




void XWEB__HttpSvrSendErr400( XWEB__clientInfo_t *pClient)
{
    const char *strErr = "HTTP/1.1 400 Bad Request\r\n"
                         "Connection: close\r\n"
                         "Content-Length: 11\r\n\r\nBad Request";
    send(pClient->socket, strErr, strlen(strErr), 0);
    XWEB__HttpSvrDropClient(pClient);
}




void XWEB__HttpSvrSendErr404( XWEB__clientInfo_t *pClient)
{
    const char *strErr = "HTTP/1.1 404 Not Found\r\n"
                         "Connection: close\r\n"
                         "Content-Length: 9\r\n\r\nNot Found";
    send(pClient->socket, strErr, strlen(strErr), 0);
    XWEB__HttpSvrDropClient(pClient);
}




void XWEB__HttpSvrServeResource( XWEB__clientInfo_t *pClient, const char *path)
{
    /*>
     * note that it is expected that the directory from which files are served
     * is name public 
     */
    char    bufFullPath[128];
    char   *strDirName = "public";
    FILE   *pFile = NULL; 
    size_t  fileSz = 0;
    const char *contentType = NULL;
    char        buf[XWEB__BUF_SZ_FILE];

    memset((void *)bufFullPath, 0, 128);
    printf("server resource: %s %s\n", XWEB__HttpSvrGetClientAddr(pClient), path);

    /* Check that client request for default page index.html */
    if (strcmp(path, "/") == 0)
    {
        path = "/index.html";
    }
    /* Check that client request file path is not greater than 100 */
    if (strlen(path) > 100)
    {
        XWEB__HttpSvrSendErr400(pClient);
        return ;
    }
    /* Check that client did not attempt to access parent directory */
    if (strstr(path, ".."))
    {
        XWEB__HttpSvrSendErr404(pClient);
        return ;
    }

    /* create the path to find file requested by client's request */
    sprintf(bufFullPath, "%s%s", strDirName, path);

    /* Check if the file requested by client exist */
    pFile = fopen(bufFullPath, "rb");
    if (!pFile)
    {
        XWEB__HttpSvrSendErr404(pClient);
        return ;
    }
    /* Determine the size of the file */
    fseek(pFile, 0L, SEEK_END);
    fileSz = ftell(pFile);
    rewind(pFile);

    /* Determine the content type of the file */
    contentType = XWEB__HttpGetContentType(bufFullPath);

    /* Create response */
    sprintf(buf, "HTTP/1.1 200 OK\r\n");
    send(pClient->socket, buf, strlen(buf), 0);

    sprintf(buf, "Connection: close\r\n");
    send(pClient->socket, buf, strlen(buf), 0);
    
    sprintf(buf, "Content-Length: %lu\r\n", fileSz);
    send(pClient->socket, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type: %s\r\n", contentType);
    send(pClient->socket, buf, strlen(buf), 0);

    sprintf(buf, "\r\n");
    send(pClient->socket, buf, strlen(buf), 0);

    int r = fread(buf, 1, XWEB__BUF_SZ_FILE, pFile);
    while (r)
    {
        send(pClient->socket, buf, r, 0);
        r = fread(buf, 1, XWEB__BUF_SZ_FILE, pFile);
    }
    fclose(pFile);
    XWEB__HttpSvrDropClient(pClient);
}



/* Prompt the user for input and retrieve it */
void  XWEB__GetInput(const char *prompt, char *buffer)
{
    printf("%s", prompt);
    buffer[0] = 0; /* null terminate */
    fgets(buffer, XWEB__MAX_INPUT, stdin);
    const int readCnt = strlen(buffer);

    if (readCnt > 0)
    {
        /* update the null terminaton afterwards */
        buffer[ readCnt - 1] = 0; 
    }
}




void  XWEB__SendFormat(socket_t fdSrv, const char *text, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, text);
    vsprintf(buf, text, args);
    va_end(args);

    send(fdSrv, buf, strlen(buf), 0);
    printf("C: %s", buf);
}


/*>
 * This methods retrieves the response code from the server which is usually 
 * a 3 digit code at the start of the response string. */
int  XWEB__StmpParseResponse(const char *response)
{
    int retCode = 0;

    const char *r = response;
    /* Check that these are not null terminator */
    if (!r[0] || !r[1] || !r[2])
    {
        goto labelExit;
    }

    for (; r[3]; r++)
    {
        if (r == response || r[-1] == '\n')
        {
            if (isdigit(r[0]) && isdigit(r[1]) && isdigit(r[2]))
            {
                if (r[3] != '-')
                {
                    if (strstr(r, "\r\n"))
                    {
                        retCode = strtol(r, 0, 10);
                    }
                }
            }
        }
    }

labelExit:
    return (retCode);
}



/* This function polls until a desired response is received from 
 * stmp server */
void  XWEB__SmtpWaitOnResp(socket_t fdSrv, int expectedCode)
{
    char resp[ XWEB__SMTP_MAX_RESPONSE + 1 ];
    char *p = resp;
    char *end = resp + XWEB__SMTP_MAX_RESPONSE;
    int code = 0;
    int bytesRecvd = 0;

    do
    {
        bytesRecvd = recv(fdSrv, p, end-p, 0);
        if (bytesRecvd < 1)
        {
            fprintf(stderr, "Connection dropped.\n");
            exit(1);
        }
        p += bytesRecvd;

        if ( p >= end)
        {
            fprintf(stderr, "Server response is too large:\n");
            fprintf(stderr, "%s", resp);
            exit(2);
        }

        *p = 0; /* Null terminate the received response */
        code = XWEB__StmpParseResponse(resp);
    }
    while ( code == 0 );

    if (code != expectedCode)
    {
        fprintf(stderr, "Error from server:\n");
        fprintf(stderr, "%s", resp);
        exit(3);
    }
    printf("S: %s", resp);
}




void XWEB__HttpsTxRequest( SSL *ssl, char *hostname, char *port, char *path)
{
    char buffer [2048];

    memset((void *)buffer, 0, 2048);
    sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
    sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
    sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    sprintf(buffer + strlen(buffer), "User-Agent: honpwc xweb 1.0\r\n");
    sprintf(buffer + strlen(buffer), "\r\n");

    SSL_write(ssl, buffer, strlen(buffer));
    printf ("Sent Headers:\n%s", buffer);
    return;
}
