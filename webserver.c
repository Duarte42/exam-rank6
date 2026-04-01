#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <sys/select.h>

int g_id = 0;
int ids[1024];
char *msgs[1024];

int max_fd = 0;
int sock_fd = 0;

fd_set afds, wfds, rfds;

char buf_read[1001], buf_write[1001];

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}


void fatal_error(void){

    write(2, "fatal error\n", 12);
    exit(1);
}

void send_all(int skip_fd, char *str){

    int fd = 0;
    while(fd <= max_fd){

        if(FD_ISSET(fd, &afds) && FD_ISSET(fd, &wfds) && fd != skip_fd && fd != sock_fd)
            send(fd, str, strlen(str), 0);

        fd++;    
       }
}

void add_user(int fd){

    FD_SET(fd, &afds);
    ids[fd] = g_id++;
    msgs[fd] = NULL;
    if(fd > max_fd)
        max_fd = fd;

    sprintf(buf_write, "server: user %d\n", ids[fd]);
    send_all(fd, buf_write);
}

