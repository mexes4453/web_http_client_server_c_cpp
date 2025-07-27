# Makefile for flutter project

# ========
# VARIABLE
# ========
BASHRC=~/.bashrc
#if( DEV )
TARGET=prog.out

GCC=gcc
SRC__GEN= xweb.c
SRC__HTTP_CLI_GET= xweb_main_http_get.c 
SRC__HTTP_SVR= xweb_main_http_svr.c 
SRC__HTTPS_CLI= xweb_main_https_cli.c 
SRC__SMTP_SEND= xweb_main_smtp_send.c 
SRC__OPENSSL_VERS= xweb_openssl_version.c 

INCLUDES= -I ./


LIB_OPENSSL= -lcrypto -lssl
# =======
# RECIPES
# =======
# remove all object files
http_cli_get: $(SRC__GEN) $(SRC__HTTP_CLI_GET)
	@echo "compiling http_cli_get"
	$(GCC) $^ $(INCLUDES) -o $(TARGET)

https_cli: $(SRC__GEN) $(SRC__HTTPS_CLI)
	@echo "compiling https client program"
	$(GCC) $^ $(INCLUDES) $(LIB_OPENSSL) -o $(TARGET)

http_svr: $(SRC__GEN) $(SRC__HTTP_SVR)
	@echo "compiling http server program"
	$(GCC) $^ $(INCLUDES) -o $(TARGET)

smtp: $(SRC__GEN) $(SRC__SMTP_SEND)
	@echo "compiling SMTP send program"
	$(GCC) $^ $(INCLUDES) -o $(TARGET)

openssl_vers: $(SRC__OPENSSL_VERS)
	@echo "compiling openssl version program"
	$(GCC) $^ $(INCLUDES) $(LIB_OPENSSL) -o $(TARGET) 
# Remove old build files
clean:
	@rm -rf $(TARGET) 2> /dev/null

# Remove old build files
fclean: clean

re: fclean

git_commit: fclean
	@git add .
	@git commit



.PHONY : all fclean clean re git_commit
