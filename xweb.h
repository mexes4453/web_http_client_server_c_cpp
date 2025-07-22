#ifndef XWEB_H
# define XWEB_H
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <netdb.h>
# include <unistd.h>
# include <time.h>
# include <arpa/inet.h>
# include <errno.h>

#define XWEB__PROTO_DELIMITER "://"
#define XWEB__CHAR_NULL       '\0'
#define XWEB__PROTO_HTTP      "http"
#define XWEB__TIMEOUT         (5)
#define XWEB__BUF_SZ_RESP     (8192)
#define XWEB__BUF_SZ_REQ      (2047)
#define XWEB__BUF_SZ_FILE     (1024)
#define XWEB__SMTP_MAX_RESPONSE (256)

enum  httpHdr_e
{
    XWEB__enHttpHdr_Length = 0,
    XWEB__enHttpHdr_Chunked,
    XWEB__enHttpHdr_Connection,
};


typedef int socket_t;
typedef struct XWEB__clientInfo_s
{
    socklen_t                  addrLen; /**< Address length */
    struct sockaddr_storage    addr;    /**< client's address  */
    socket_t                   socket;  /**< client's socket   */

    /**< store receivd clients request */
    char                       request[ XWEB__BUF_SZ_REQ ]; 

    /**< Nbr of bytes recevd as client request */
    int                        recvdBytes;
    struct XWEB__clientInfo_s *pNext; /**< Client info of next client */
}   XWEB__clientInfo_t;




/* http client */
int         XWEB__HttpParseUrl( char *url, char **hostname, char **port, char **path);
int         XWEB__HttpSendRequest( socket_t sfd, char *hostname, char *port, char *path);
int         XWEB__HttpConnectToHost( socket_t *sfd, char *hostname, char *port);
const char *XWEB__HttpGetContentType( const char *path);




/* http server */
XWEB__clientInfo_t * XWEB__HttpSvrGetClientsList(void);
int         XWEB__HttpSvrCreateSock( socket_t *sfd, char *hostname, char *port);
int         XWEB__HttpSvrGetClient( socket_t sfd, XWEB__clientInfo_t **pClientAddr);
int         XWEB__HttpSvrDropClient( XWEB__clientInfo_t *pClient);
const char *XWEB__HttpSvrGetClientAddr( XWEB__clientInfo_t *pClient);
fd_set      XWEB__HttpSvrWaitOnClients( socket_t srv);
void        XWEB__HttpSvrSendErr400( XWEB__clientInfo_t *pClient);
void        XWEB__HttpSvrSendErr404( XWEB__clientInfo_t *pClient);
void        XWEB__HttpSvrServeResource( XWEB__clientInfo_t *pClient, const char *path);


/* smtp - mail */
void       XWEB__GetInput(const char *prompt, char *buffer);


#endif /* XWEB_H */