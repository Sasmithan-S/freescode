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
#include <string.h>
#include "list/list.h"
#include "user.h"
#include "utils.h"
#define PORT_FREESCORD 4321

/** Gérer toutes les communications avec le client renseigné dans
 * user, qui doit être l'adresse d'une struct user */
void *handle_client(void *user);
/** Créer et configurer une socket d'écoute sur le port donné en argument
 * retourne le descripteur de cette socket, ou -1 en cas d'erreur */
int create_listening_sock(uint16_t port);


void *repeteur(void * t);
int tube[2];
pthread_mutex_t verrou;
struct list *liste_clt;
int main(int argc, char *argv[])
{	
	pthread_mutex_init(&verrou,NULL);
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
	ssize_t n;
	char buf[256];
	while((n=read(tube[0],buf,sizeof(buf)))>0){
		pthread_mutex_lock(&verrou);
		struct node *tmp = liste_clt->first;
		while(tmp!=NULL){
			struct user* c = (struct user*)tmp->elt;
			write(c->sock,buf,n);
			tmp=tmp->next;
		}
		pthread_mutex_unlock(&verrou);
	}
	return NULL;
}




/** Gérer toutes les communications avec le client renseigné dans
    * user, qui doit être ladresse dune struct user */
void *handle_client(void *clt)
{
	struct user *client = (struct user *)clt;
	char buf[512];
	int bon = 0;
	char *msg = "Bienvenu !\r\nEcrivez: nickname suivi de votre nom \r\n";

	write(client->sock, msg,strlen(msg));
	while (!bon){
		if (buff_fgets_crlf(client->b,buf,sizeof(buf))==NULL){
			close(client->sock);
			user_free(client);
			return NULL;
		}
		crlf_to_lf(buf);
		int t = strlen(buf);
		if(t >0 && buf[t -1] == '\n'){
			buf[t-1]= '\0';
		}
		if (strncmp(buf,"nickname ",9) != 0){
			write(client->sock,"3 \r\n",4);
			continue;
		} 
		char * nom = buf+9;
		if(strlen(nom)>16){
			write(client->sock, "2 \r\n",4);
			continue;
		}
		int dispo = 0;
		pthread_mutex_lock(&verrou);
		struct node *tmp = liste_clt->first;
		while(tmp != NULL){
			struct user *c = (struct user*)tmp->elt;
			if(strcmp(c->nickname,nom) == 0){
				dispo = 1;
				break;
			} tmp=tmp->next;
		}
		pthread_mutex_unlock(&verrou);
		if (dispo){
			write(client->sock,"1 \r\n",4);
			continue;
		}
		strncpy(client->nickname,buf+9,16);
		client->nickname[16] = '\0';
		write(client->sock,"0 \r\n",4);
		bon = 1;
	}
	pthread_mutex_lock(&verrou);
	list_add(liste_clt,client);
	pthread_mutex_unlock(&verrou);

	while(buff_fgets_crlf(client->b,buf,sizeof(buf))!=NULL){
		char c[1024];
		int l_msg = strlen(buf);
		int l_nom = strlen(client->nickname);
		memcpy(c,client->nickname,l_nom);
		memcpy(c + l_nom,": ",2);
		memcpy(c+l_nom+2,buf,l_msg);
		write(tube[1],c,2+l_msg+l_nom);
	}
	pthread_mutex_lock(&verrou);
	list_remove_element(liste_clt,client);
	pthread_mutex_unlock(&verrou);
	close(client->sock);
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


