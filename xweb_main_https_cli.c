#include "xweb.h"

int main(int argc, char *argv[])
{
    SSL_CTX *ctx;
    SSL     *ssl;
    int      retCode = 0;
    char    *hostname;
    char    *port;
    socket_t fdSockSvr;
    char    *tmp;
    X509    *cert;
    int      bytesRecvd = 0;
    char     buffer[2048];

    /*>
     * Initialise the ssl library */
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings(); // for verbose utility on error

    if (argc < 3)
    {
        fprintf(stderr, "usage: prog.out hostname port\n");
        retCode = 1;
        goto labelExit;
    }

    /*>
     * Create a context instance for the ssl/tls */
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx)
    {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        retCode = 2;
        goto labelExit;
    }
    
    /* connect to server */
    hostname = argv[1];
    port = argv[2];
    retCode = XWEB__HttpConnectToHost(&fdSockSvr, hostname, port);
    if (retCode) XWEB__ERR("HttpConnectToHost(...) failed\n", 3, &retCode, labelExit);


    /*>
     * Initiate ssl/tls connection with the created context
     * This can be omitted if the encryption is not required */
    ssl = SSL_new(ctx);
    if (!ssl) XWEB__ERR("SSL_new() failed.\n", 4, &retCode, labelExit);

    /*>
     * Set the hostname to the ssl connection instance */
    if (!SSL_set_tlsext_host_name(ssl, hostname))
    {
        ERR_print_errors_fp(stderr);
        XWEB__ERR("fail to set hostname to ssl\n", 5, &retCode, labelExit);
    }
    
    /*>
     * Set the socket to the ssl connection instance */
    SSL_set_fd(ssl, fdSockSvr);

    /*>
     * Establish connection */
    retCode = SSL_connect(ssl);
    if (retCode == -1)
    {
        ERR_print_errors_fp(stderr);
        XWEB__ERR("SSL_connect() failed.\n", 6, &retCode, labelExit);
    }

    /*>
     * Print the selected cipher suite  */
    printf("SSL/TLS using %s\n", SSL_get_cipher(ssl));

    /*>
     * Retrieve the server certificate */
    cert = SSL_get_peer_certificate(ssl);
    if (!cert) XWEB__ERR("SSL_get_peer_cert..() failed.\n", 7, &retCode, labelExit);

    /* Print the certificate subject */
    tmp = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    if (tmp)
    {
        printf("subject: %s\n", tmp);
        OPENSSL_free(tmp);
        tmp = NULL;
    }

    /* Print the certificate subject */
    tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    if (tmp)
    {
        printf("issuer: %s\n", tmp);
        OPENSSL_free(tmp);
        tmp = NULL;
    }
    X509_free(cert);

    XWEB__HttpsTxRequest(ssl, hostname, port, "\0");

    while (1)
    {
        bytesRecvd = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytesRecvd < 1)
        {
            printf("\nConnection closed by peer.\n");
            break ;
        }
        printf("Received (%d bytes): '%.*s'\n", bytesRecvd, bytesRecvd, buffer);
    }

labelExit:
    printf("\nClosing socket...\n");
    SSL_shutdown(ssl);
    close(fdSockSvr);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    printf("Finished.\n");
    return (retCode);
}


