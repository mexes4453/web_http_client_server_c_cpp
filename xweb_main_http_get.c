
#include "xweb.h"



int main(int argc, char *argv[])
{
    char     *strURL;
    char     *pHostname = NULL;
    char     *pPort = NULL;
    char     *pPath = NULL;
    int       retCode = 0;
    char      response[XWEB__BUF_SZ_RESP + 1];
    char      *p = response, *q;
    char      *end = response + XWEB__BUF_SZ_RESP;
    char      *body = 0;
    int       encoding = 0;
    int       remaining = 0;
    socket_t  serverFd = 0;
    clock_t   startTime = 0;
    fd_set    reads;
    struct timeval timeout;
    int            bytesRecvd = 0;


    if (argc < 2)
    {
        fprintf(stderr, "usage: xweb url\n");
        retCode = 5;
        goto labelExit;
    }
    strURL = argv[1];
    
    
    retCode = XWEB__HttpParseUrl(strURL, &pHostname, &pPort, &pPath);
    if (retCode != 0)
    {
        goto labelExit;
    }

    retCode = XWEB__HttpConnectToHost(&serverFd, pHostname, pPort);
    if (retCode != 0)
    {
        goto labelExit;
    }
    printf("connect to host\n");

    retCode = XWEB__HttpSendRequest(serverFd, pHostname, pPort, pPath);
    if (retCode != 0)
    {
        goto labelExit;
    }
    printf("sendRequest\n");

    startTime = clock();
    while (1)
    {
        printf("wait recev\n");
        /* check that there is no time out in waitng for response */
        if ( ((clock() - startTime) / CLOCKS_PER_SEC) > XWEB__TIMEOUT)
        {
           fprintf(stderr, "timeout \n");
           retCode = 10;
           goto labelExit;
        }

        /* check that there is enough space to recieve response */
        if (p == end)
        {
           fprintf(stderr, "out of buffer space \n");
           retCode = 11;
           goto labelExit;
        }

        /* initialise file descriptor sets */
        FD_ZERO(&reads);
        FD_SET(serverFd, &reads);
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;

        if (select(serverFd+1, &reads, 0,0, &timeout) < 0)
        {
           fprintf(stderr, "select failed \n");
           retCode = 12;
           goto labelExit;
        }

        if (FD_ISSET(serverFd, &reads))
        {
            bytesRecvd = recv(serverFd, p, end-p, 0);
            if (bytesRecvd < 1)
            {
                if ( encoding == XWEB__enHttpHdr_Connection && body)
                {
                    printf( "%.*s", (int)(end - body), body);
                }
                printf("\nConnection closed by peer.\n");
                break ;
            }
            p += bytesRecvd;
            *p = 0;

            /* Find the end of header and start of body in response */
            if (!body && (body = strstr(response, "\r\n\r\n")))
            {
                *body = 0;
                body += 4;
                printf("Received Headers:\n%s\n", response);
            }

            q = strstr(response, "\nContent-Length: ");
            if (q)
            {
                encoding = XWEB__enHttpHdr_Length;
                q = strchr(q, ' ');
                q +1;
                remaining = strtol(q, 0, 10);
            }
            else
            {
                q = strstr(response, "\nTransfer-Encoding: chunked ");
                if (q)
                {
                    encoding = XWEB__enHttpHdr_Chunked;
                    remaining = 0;
                }
                else
                {
                    encoding = XWEB__enHttpHdr_Connection;
                }
            }

            /*>
             * At this point, the body pointer has been updated by one of the
             * sections above. Hence, this section would process the body 
             * based on how the servers notifies the client of the body size
             * which can be determined by the updated encoding variable. */
            printf("\nReceived Body:\n");
            if (body)
            {
                if (encoding == XWEB__enHttpHdr_Length)
                {
                    if (p - body >= remaining)
                    {
                        printf("%.*s", remaining, body);
                        break ;
                    }
                }
                else if (encoding == XWEB__enHttpHdr_Chunked)
                {
                    do 
                    {
                        if (remaining == 0)
                        {
                            if ( (q = strstr(body, "\r\n"))) 
                            {
                                remaining = strtol(body, 0, 16);
                                if (!remaining) goto labelExit;
                                body = q + 2;
                            }
                            else break;
                        }
                        if ( remaining && (p -body) >= remaining)
                        {
                            printf("%.*s", remaining, body);
                            body += remaining + 2;
                            remaining = 0;
                        }
                    } while (!remaining);
                }
            }
        }
    }


    retCode = 0;
labelExit:
    if (serverFd) close(serverFd);
    printf("Finished. retCode( %d )\n", retCode);
    return (retCode);
} 


