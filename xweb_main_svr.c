
#include "xweb.h"



int main(int argc, char *argv[])
{
    int                 retCode = 0;
    socket_t            serverFd = 0;
    fd_set              reads;
    int                 bytesRecvd = 0;
    XWEB__clientInfo_t *pClient=NULL; 
    char               *q = NULL;
    char               *pPathStart = NULL;
    char               *pPathEnd = NULL;


    retCode = XWEB__HttpSvrCreateSock(&serverFd, "127.0.0.1", "8080");
    if (retCode)
    {
        fprintf(stderr, "Error: create sock failed \n");
        retCode = 40;
        goto labelExit;
    }
    while (1)
    {
        /*>
         * Check for a new client to connect to server or and existing client to
         * send new data */
        reads = XWEB__HttpSvrWaitOnClients(serverFd);


        if (FD_ISSET(serverFd, &reads))
        {
            retCode = XWEB__HttpSvrGetClient(-1, &pClient);
            {
                pClient->socket = accept(serverFd, 
                                         (struct sockaddr *)&(pClient->addr),
                                        &(pClient->addrLen));
                if ( pClient->socket < 0)
                {
                    fprintf(stderr, "Error: accept failed \n");
                    retCode = 41;
                    goto labelExit;
                }
                printf("New connection from %s.\n", 
                        XWEB__HttpSvrGetClientAddr(pClient));
            }
        }

        pClient = XWEB__HttpSvrGetClientsList();
        while (pClient)
        {
            if (FD_ISSET(pClient->socket, &reads))
            {
                if ( XWEB__BUF_SZ_REQ == pClient->recvdBytes)
                {
                    XWEB__HttpSvrSendErr400(pClient);
                    continue ;
                }
                bytesRecvd= recv( pClient->socket, 
                              pClient->request + pClient->recvdBytes,
                              XWEB__BUF_SZ_REQ - pClient->recvdBytes, 0);
                if (bytesRecvd < 1)
                {
                    printf("Unexpected disconnect from %s.\n", 
                            XWEB__HttpSvrGetClientAddr(pClient));
                    XWEB__HttpSvrDropClient(pClient);

                }
                else
                {
                    pClient->recvdBytes += bytesRecvd;
                    pClient->request[ pClient->recvdBytes ] = 0; /* null termination */

                    /* Check that the recieved request was terminated with 2 blank lines */
                    q = strstr(pClient->request, "\r\n\r\n");
                    if (q)
                    {
                        /* Check the request has a valid path that starts with '/' */
                        if (strncmp("GET /", pClient->request, 5))
                        {
                            XWEB__HttpSvrSendErr400(pClient);
                        }
                        else
                        {
                            pPathStart = pClient->request + 4;
                            pPathEnd = strstr(pPathStart, " ");
                            if (!pPathEnd)
                            {
                                XWEB__HttpSvrSendErr400(pClient);
                            }
                            else
                            {
                                *pPathEnd = 0;
                                XWEB__HttpSvrServeResource(pClient, pPathStart);
                            }
                        }
                    }
                }
            }
            pClient = pClient->pNext;
        }
    }
    retCode = 0;
labelExit:
    if (serverFd) close(serverFd);
    printf("Finished. retCode( %d )\n", retCode);
    return (retCode);
} 


