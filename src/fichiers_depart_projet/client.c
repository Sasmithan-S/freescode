/* Sasmithan Satkunarajah 12403246
        Je déclare quil sagit de mon propre travail.
        Ce travail a été réalisé intégralement par un être humain. */

#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include "buffer/buffer.h"
#include "utils.h"
#include <string.h>
#define PORT_FREESCORD 4321

/** se connecter au serveur TCP d'adresse donnée en argument sous forme de
 * chaîne de caractère et au port donné en argument
 * retourne le descripteur de fichier de la socket obtenue ou -1 en cas
 * d'erreur. */
int connect_serveur_tcp(char *adresse, uint16_t port);

int main(int argc, char *argv[])
{	
	if (argc <2 ){
		printf("Pas assez d'arguments");
		return 1;

	}
	int sock_cl = connect_serveur_tcp(argv[1], PORT_FREESCORD);
	if (sock_cl <0) {
		return 1;
	}

	struct pollfd fds[2] = {
		{.fd = 0 , .events = POLLIN},
		{.fd = sock_cl, .events = POLLIN }
	};
	buffer * b = buff_create(sock_cl,512);
	if(b == NULL){
		printf("erreur creation buffer");
		close(sock_cl);
		return 1;
	}
	while (1){

		if (buff_ready(b)){
			fds[0].revents = 0;
			fds[1].revents = 0;

		} else { if (poll(fds,2,-1)<=0){
			printf("erreur/fin poll");
			break;
		}
		
		}
		char t[512];
		ssize_t n;
		//utilisateur
		if(fds[0].revents & (POLLIN | POLLHUP )){
				if((n = read(0, t,sizeof(t)-2))>0){
					t[n]= '\0';
					lf_to_crlf(t);
					write(sock_cl,t,strlen(t));
				}
				else {
					break;
				}
		}
		//serv
		if (fds[1].revents & (POLLIN | POLLHUP) || buff_ready(b)){
			if(buff_fgets_crlf(b,t,sizeof(t))!=NULL){
				crlf_to_lf(t);
				printf("%s",t);
			}
			else {
				if(buff_eof(b)){
					break;
				}
			}
		}
	}
	buff_free(b);
	close(sock_cl);
	return 0;
}








int connect_serveur_tcp(char *adresse, uint16_t port)
{	
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if (sock<0) {
		perror("erreur socket client");
		return -1;
	}
	struct sockaddr_in sa = {.sin_family = AF_INET, .sin_port = htons(port)};
	//conversion adr ip en adr eseau binaire
	if (inet_pton(AF_INET, adresse, &sa.sin_addr) != 1){
		perror ("erreur ip");
		return -1;
 
	}
	socklen_t sl = sizeof(sa);
	if (connect(sock, (struct sockaddr *) &sa,sl ) < 0){
		perror("erreur connect");
		exit(3);
	}
		return sock;
}
