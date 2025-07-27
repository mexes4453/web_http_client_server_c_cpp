/*>
 * Altenatively, the commandline tool swaks can be used for this purpose
 *
 *
 * $ swaks --to chimexes@gmail.com  \ 
 *         --from chiemezieudoh@gmail.com \ 
 *         --server smtp.gmail.com \ 
 *         --port 587 \ 
 *         --auth LOGIN \ 
 *         --auth-user chiemezieudoh@gmail.com \ 
 *         --auth-password '<app generated password' \ // This password is not 
 *                                                     // your usual google login password
 *                                                     // but one generated from your mail
 *                                                     // service provider (apps password)
 *                                                     // it consist of 16digit characters.
 *         --tls  \
 *         --data "Subject: Testmail\n\nTrying sending mail from the command line."
 * 
 * command output 
=== Trying smtp.gmail.com:587...
=== Connected to smtp.gmail.com.
<-  220 smtp.gmail.com ESMTP ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
 -> EHLO mexes
<-  250-smtp.gmail.com at your service, [2a02:8071:4586:a540:58cd:ed39:e7d7:959c]
<-  250-SIZE 35882577
<-  250-8BITMIME
<-  250-STARTTLS
<-  250-ENHANCEDSTATUSCODES
<-  250-PIPELINING
<-  250-CHUNKING
<-  250 SMTPUTF8
 -> STARTTLS
<-  220 2.0.0 Ready to start TLS
=== TLS started with cipher TLSv1.3:TLS_AES_256_GCM_SHA384:256
=== TLS no local certificate set
=== TLS peer DN="/CN=smtp.gmail.com"
 ~> EHLO mexes
<~  250-smtp.gmail.com at your service, [2a02:8071:4586:a540:58cd:ed39:e7d7:959c]
<~  250-SIZE 35882577
<~  250-8BITMIME
<~  250-AUTH LOGIN PLAIN XOAUTH2 PLAIN-CLIENTTOKEN OAUTHBEARER XOAUTH
<~  250-ENHANCEDSTATUSCODES
<~  250-PIPELINING
<~  250-CHUNKING
<~  250 SMTPUTF8
 ~> AUTH LOGIN
<~  334 VXNlcm5hbWU6
 ~> Y2hpZW1lemlldWRvaEBnbWFpbC5jb20=
<~  334 UGFzc3dvcmQ6
 ~> dnR6cXFpa3lxZmdzbnZjdw==
<~  235 2.7.0 Accepted
 ~> MAIL FROM:<chiemezieudoh@gmail.com>
<~  250 2.1.0 OK ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
 ~> RCPT TO:<chimexes@gmail.com>
<~  250 2.1.5 OK ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
 ~> DATA
<~  354 Go ahead ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
 ~> Subject: Testmail
 ~> 
 ~> Trying sending mail from the command line.
 ~> .
<~  250 2.0.0 OK  1753522157 ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
 ~> QUIT
<~  221 2.0.0 closing connection ffacd0b85a97d-3b778f047afsm2210013f8f.39 - gsmtp
=== Connection closed with remote host.
 */


#include "xweb.h"



int main(int argc, char *argv[])
{
    char      hostname[ XWEB__MAX_INPUT];
    socket_t  serverFd = 0;
    int       retCode = 0;
    char      sender[ XWEB__MAX_INPUT];
    char      receipent[ XWEB__MAX_INPUT];
    char      subject[ XWEB__MAX_INPUT];
    char      body[ XWEB__MAX_INPUT];
    char      date[128];
    time_t    timer;
    struct tm *timeinfo;
#if 0
    char     *pPort = NULL;
    char     *pPath = NULL;
    char      response[XWEB__BUF_SZ_RESP + 1];
    char      *p = response, *q;
    char      *end = response + XWEB__BUF_SZ_RESP;
    char      *body = 0;
    clock_t   startTime = 0;
    fd_set    reads;
    struct timeval timeout;
    int            bytesRecvd = 0;
#endif
    memset(hostname, 0, XWEB__MAX_INPUT);
    XWEB__GetInput("Enter hostname: ", hostname);
    printf("Connecting to host: %s:25", hostname);
    retCode = XWEB__HttpConnectToHost(&serverFd, hostname, "25");
    if (retCode != 0)
    {
        fprintf(stderr, "failed to connect to host\n");
        retCode = 5;
        goto labelExit;
    }

    /* Wait for server to say that is its ready */
    XWEB__SmtpWaitOnResp(serverFd, 220);

    /* Send helo to server and wait for its response */
    XWEB__SendFormat(serverFd, "HELO HONPWC\r\n");
    XWEB__SmtpWaitOnResp(serverFd, 250);

    /* send the sender info to server */
    XWEB__GetInput("sender: ", sender);
    XWEB__SendFormat(serverFd, "MAIL FROM:<%s>\r\n", sender);
    XWEB__SmtpWaitOnResp(serverFd, 250);

    /* send the receiver info to server */
    XWEB__GetInput("receiver: ", receipent);
    XWEB__SendFormat(serverFd, "RCPT TO:<%s>\r\n", receipent);
    XWEB__SmtpWaitOnResp(serverFd, 250);


    /* Instruct the server to listen to actual email */
    XWEB__SendFormat(serverFd, "DATA\r\n");
    XWEB__SmtpWaitOnResp(serverFd, 354);


    XWEB__GetInput("subject: ", subject);
    XWEB__SendFormat(serverFd, "From:<%s>\r\n", sender);
    XWEB__SendFormat(serverFd, "To:<%s>\r\n", receipent);
    XWEB__SendFormat(serverFd, "Subject:<%s>\r\n", subject);

    /* get date and time */
    time(&timer);
    timeinfo = gmtime(&timer);
    strftime(date, 128, "%a, %d %b %Y $H:%M:%s +0000", timeinfo);
    XWEB__SendFormat(serverFd, "Date:<%s>\r\n", date);
    XWEB__SendFormat(serverFd, "\r\n");

    printf("Enter your email text, end with \".\" on a line by itself.\n");
    
    while (1)
    {
        XWEB__GetInput("> ", body);
        XWEB__SendFormat(serverFd, "%s\r\n", body);
        if ( strcmp(body, ".") == 0)
        {
            break ;
        }
    }
    XWEB__SmtpWaitOnResp(serverFd, 250);
    XWEB__SendFormat(serverFd, "QUIT\r\n");
    XWEB__SmtpWaitOnResp(serverFd, 221);


    retCode = 0;
labelExit:
    if (serverFd)
    {
        printf("\nClosing socket...\n");
        close(serverFd);
    }
    printf("Finished. retCode( %d )\n", retCode);
    return (retCode);
} 


