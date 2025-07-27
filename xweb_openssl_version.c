
#include <openssl/ssl.h> //sudo apt install libssl-dev
#include <stdio.h>

int main(void)
{
    printf("Openssl Version: %s\n", OpenSSL_version(SSLEAY_VERSION));
    return (0);
}