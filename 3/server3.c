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
	int my_fd,client_fd;
	struct sockaddr_in server, client;
	int client_size;
	int error = 0, wrote = 0;
	char buffer[] = "Hello there! Welcome to the SSL test server.\n\n";
	const SSL_METHOD *my_ssl_method;
	SSL_CTX *my_ssl_ctx;
	SSL *my_ssl;
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	my_ssl_method = TLS_server_method();
	
	if( ( my_ssl_ctx = SSL_CTX_new(my_ssl_method) ) == NULL ) {
		ERR_print_errors_fp(stderr);
		exit(-1);
	}
	SSL_CTX_use_certificate_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);
	SSL_CTX_use_PrivateKey_file(my_ssl_ctx,"server.pem",SSL_FILETYPE_PEM);
	if( !SSL_CTX_check_private_key(my_ssl_ctx) ) {
		fprintf(stderr,"Private key does not match certificate\n");
		exit(-1);
	}
	my_fd = socket(PF_INET, SOCK_STREAM, 0);
	
	server.sin_family = AF_INET;
	server.sin_port = htons(5353);
	server.sin_addr.s_addr = INADDR_ANY;
	
	bind(my_fd, (struct sockaddr *)&server, sizeof(server));
	
	listen(my_fd, 5);
	
	for( ;; ) {
		client_size = sizeof(client);
		bzero(&client,sizeof(client));
		client_fd = accept(my_fd, (struct sockaddr *) &client, (socklen_t*)&client_size);
		if((my_ssl = SSL_new(my_ssl_ctx)) == NULL) {
			ERR_print_errors_fp(stderr);
			exit(-1);
		}
		
		SSL_set_fd(my_ssl,client_fd);
		if(SSL_accept(my_ssl) <= 0) {
			ERR_print_errors_fp(stderr);
			exit(-1);
		}
		printf("[%s,%s]\n",SSL_get_version(my_ssl),SSL_get_cipher(my_ssl));
		for(wrote = 0; wrote < strlen(buffer); wrote += error) {
			error = SSL_write(my_ssl,buffer+wrote,strlen(buffer)-wrote);
			if(error <= 0)
			break;
		}
		SSL_shutdown(my_ssl);
		SSL_free(my_ssl);
		close(client_fd);
	}
	SSL_CTX_free(my_ssl_ctx);
}
