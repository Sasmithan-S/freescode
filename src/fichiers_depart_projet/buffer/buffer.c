#include "buffer.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct buffer {
	int fd; //descripteur fichier
	char *buff; //buffer
	size_t bufsz; //taille max
	char *last_read_char;
	char *next_char;
	int eof; //0 si read()>0 , 1 sinon
};

buffer *buff_create(int fd, size_t buffsz)
{
	struct buffer *b = malloc(sizeof(struct buffer));
	if (b == NULL) return NULL;
	b->buff = malloc(buffsz);
	if (b->buff == NULL) {
		free(b);
		return NULL;
	}
	b->bufsz = buffsz;
	b->fd = fd;
	b->eof = 0;
	b->last_read_char = NULL;
	b->next_char = NULL;
	return b;
}

int buff_getc(buffer *b)
{
	if (b == NULL) return EOF;
	if (b->eof == 1) return EOF;
	if (b->next_char == NULL || b->last_read_char<b->next_char){
		ssize_t n;
		n = read(b->fd,b->buff,b->bufsz);
		if (n <=0){
			b->eof = 1;
			return EOF;
		}
		b->next_char = b->buff;
		b->last_read_char = b->buff+n-1;
	}
	unsigned char c = *(b->next_char);
	b->next_char++;
	return (int)c ;
}

int buff_ungetc(buffer *b, int c)
{
	if (b == NULL || b->next_char == NULL || b->next_char == b->buff) return EOF;
	b->next_char = b->next_char -1;
	*(b->next_char) = (unsigned char ) c;
	return c;
}

void buff_free(buffer *b)
{ if (b == NULL) return;
	if(b->buff != NULL){
		free(b->buff);
	}
	free(b);

}

int buff_eof(const buffer *buff)
{
	if (buff == NULL) return 1;
	return buff->eof;
}

int buff_ready(const buffer *buff)
{
	if (buff == NULL || buff->next_char == NULL) return 0;
	return buff->next_char <= buff->last_read_char;

}

char *buff_fgets(buffer *b, char *dest, size_t size)
{
	if (b == NULL || dest == NULL || size == 0) return NULL; 
	size_t i = 0;
	while (i < size - 1){
		int c = buff_getc(b);
		if (c == EOF) break;
		dest[i] = (char)c;
		i++;
		if (c == '\n'){
			break;
		}
	}
	if (i == 0) return NULL;
	dest[i] = '\0';
	return dest;
}

char *buff_fgets_crlf(buffer *b, char *dest, size_t size)
{
	if (b == NULL || dest == NULL || size == 0) return NULL; 
	size_t i = 0;
	while (i < size - 1){
		int c = buff_getc(b);
		if (c == EOF) break;
		dest[i] = (char)c;
		i++;
			
		if (c == '\n' && i>=2 && dest[i-2] == '\r'){
			break;
		}
		
	}
	if (i == 0) return NULL;
	dest[i] = '\0';
	return dest;
}
