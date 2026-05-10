#ifndef USER_H
#define USER_H
#include <sys/socket.h>
#include <netinet/in.h>
#include "buffer/buffer.h"
struct user {
	struct sockaddr *address;
	socklen_t addr_len;
	int sock;
	/* autres champs éventuels */
	buffer *b;
	char nickname[17];
};

struct user *user_accept(int sl);
void user_free(struct user *user);

#endif /* ifndef USER_H */
