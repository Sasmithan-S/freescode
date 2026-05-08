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

	struct pollfd fds[2];
	fds[0].fd = 0;
	fds[1].fd = sock_cl;
	fds[0].events = POLLIN;
	fds[1].events = POLLIN;

	while (poll(fds,2,-1)>0){
		char buf[256];
		int n;
		//utilisateur
		if(fds[0].revents & (POLLIN )){
				if((n = read(0, buf,sizeof(buf)))>0){
					write(sock_cl,buf,n);
				}
				else {
					break;
				}
		}
		//serv
		if (fds[1].revents & (POLLIN)){
			if ((n=read(sock_cl,buf,sizeof(buf))) >0)	{
				write(1,buf,n);
			} else {
				printf(" serv deco ");
				break;
			}
		}
	}
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
