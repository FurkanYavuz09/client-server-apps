#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main(){
	const SSL_METHOD *my_ssl_method;
	SSL_CTX *my_ssl_ctx;
	SSL *my_ssl;
	int my_fd;
	struct sockaddr_in server;
	int error = 0, read_in = 0;
	char buffer[512];
	
	memset(buffer,'\0',sizeof(buffer));
	
	OpenSSL_add_all_algorithms();
	
	SSL_load_error_strings();
	
	my_ssl_method = TLS_client_method();
	
	if((my_ssl_ctx = SSL_CTX_new(my_ssl_method)) == NULL) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	
	SSL_CTX_use_certificate_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);
	
	SSL_CTX_use_PrivateKey_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);
	
	if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	my_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(5353);
	inet_aton("127.0.0.1",&server.sin_addr);
	
	bind(my_fd, (struct sockaddr *)&server, sizeof(server));
	
	connect(my_fd,(struct sockaddr *)&server, sizeof(server));
	
	printf("%s", "Connection established. ");
	
	SSL_set_fd(my_ssl,my_fd);
	
	if(SSL_connect(my_ssl) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	printf("[SSL version : %s, cipher :%s]\n",SSL_get_version(my_ssl),SSL_get_cipher(my_ssl));
	
	for( read_in = 0; read_in < sizeof(buffer); read_in += error ) {
		error = SSL_read(my_ssl,buffer+read_in,sizeof(buffer) - read_in);
		if(error <= 0)
			break;
	}
	SSL_shutdown(my_ssl);
	SSL_free(my_ssl);
	SSL_CTX_free(my_ssl_ctx);
	close(my_fd);
	printf("This is a Message coming from server  :%s",buffer);
}
