#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list/list.h"
#include "user.h"

#define PORT_FREESCORD 4321

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);
/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);


void *repeteur(void * t);
int tube[2];
struct list *liste_clt;
int main(int argc, char *argv[])
{	
	int sock_l = create_listening_sock(PORT_FREESCORD);
	if (sock_l < 0) return 1;
	if (pipe(tube)<0){
		perror("errur tube");
		return 1;
	}
	liste_clt = list_create();
	pthread_t rep_t;
	if (pthread_create(&rep_t,NULL,repeteur,NULL)!=0) return 1;
	pthread_detach(rep_t);
	
	//boucle infini
	while(1){

		struct user* cl =  user_accept(sock_l);
		if (cl == NULL){
			continue;
		}
		pthread_t t;
		if (pthread_create(&t,NULL,handle_client,cl)!=0){
			perror("erreur threads");
			user_free(cl);
			continue;
		}


	pthread_detach(t);
	}


	return 0;
}


/**  Il est en lecture sur le tube, puis, chaque
fois qu’il lit des octets dans le tube, les réécrit dans toutes les sockets de tous les utilisateurs.
*/
void *repeteur(void*t){
	int n;
	char buf[256];
	while((n=read(tube[0],buf,sizeof(buf)))>0){
		struct node *tmp = liste_clt->first;
		while(tmp!=NULL){
			struct user* c = (struct user*)tmp->elt;
			write(c->sock,buf,n);
			tmp=tmp->next;
		}
	}
	return NULL;
}




/** Gérer toutes les communications avec le client renseigné dans
    * user, qui doit être ladresse dune struct user */
void *handle_client(void *clt)
{
	struct user *client = (struct user *)clt;
	list_add(liste_clt,client);
	char buf[256];
	int n;
	while((n=read(client->sock,buf,sizeof(buf)))>0){
		write(tube[1],buf,n);
	}
	close(client->sock);
	list_remove_element(liste_clt,client);
	user_free(client);
	return NULL;

}








/** Créer et configurer une socket découte sur le port donné en argument
    * retourne le descripteur de cette socket, ou -1 en cas derreur */

int create_listening_sock(uint16_t port)
{   //création du socket
	int sock_l = socket(AF_INET,SOCK_STREAM,0);
	if (sock_l <0) {
		perror("erreur socket");
		return -1;
	}
	struct sockaddr_in sa = { .sin_family = AF_INET , .sin_port = htons(port),.sin_addr.s_addr = htonl(INADDR_ANY)};
	socklen_t sl = sizeof(sa);
	int opt = 1;
	//pour faie en sorte que l'on puisse réutiliser l'adr sans attendre la fin du serveur
	setsockopt(sock_l,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
	if (bind(sock_l,(struct sockaddr *) &sa, sl)<0)	{
		perror("erreur bind");
		return -1;
	}
	if (listen(sock_l,128)<0){
		perror("pb listen");
		return -1;
	}
	return sock_l;
}


