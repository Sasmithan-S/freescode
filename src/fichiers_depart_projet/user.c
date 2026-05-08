#include "user.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

/** accepter une connection TCP depuis la socket d'écoute sl et retourner un
 * pointeur vers un struct user, dynamiquement alloué et convenablement
 * initialisé */
struct user *user_accept(int sl)
{	struct user* nc  = malloc(sizeof(struct user));
	if (nc == NULL){
		return NULL;
	}
	nc->address = malloc (sizeof(struct sockaddr_in));
	if (nc->address == NULL){
		free(nc);
		return NULL;
	}	
	nc->addr_len = sizeof(struct sockaddr_in);
	int s = accept(sl, (struct sockaddr*)nc->address,&nc->addr_len);
	if (s<0){
		perror("erreur acceptation user");
		user_free(nc);
		return NULL;	
	}
	nc->sock = s;
	return nc;
}

/** libérer toute la mémoire associée à user */
void user_free(struct user *user)
{	if (user != NULL){
	free(user->address);
	free(user);	
	}
}
