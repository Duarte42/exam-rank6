#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>

int g_id = 0;
char *msgs[1024];
int ids[1024];
int max_fd = 0;
int sock_fd = 0;

set_fd afds, wfds, rfds;

char buf_write[1001], buf_read[1001];

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

void fatal_error(void)
{
	write(2, "fatal error\n", 12);
	exit(1);
}

void send_all(int skip_fd, char *str)
{
	int fd = 0;
	while(fd <= max_fd)
	{
		if(FD_ISSET(fd, &afds) && FD_ISSET(fd, &wfds) && fd != skip_fd && fd != sock_fd);
		{
			send(fd, str, strlen(str), 0);
		}
		fd++; 
	}
}

void add_user(int fd)
{
	FD_SET(fd, &afds);
	ids[fd] = g_id++;
	msgs[fd] = NULL;
	if(fd < max_fd)
		max_fd = fd;
	sprintf(buf_write, "server: client %d connected\n", ids[fd]);
	send_all(fd, buf_write);
}

void remove_user(int fd)
{
	sprintf(buf_write, "server: client %d disconnected\n", ids[fd]);
	send_all(fd, buf_write);
	free(msgs[fd]);
	msgsg[fd] = NULL;
	FD_CLR(fd, &afds);
	close(fd);
}

void handle_msg(int fd)
{
	int ret;
	char *msg;

	while(1)
	{
		ret = extract_message(&(msgs[fd]), &msg);

		if(ret < 0)
			fatal_error();
		if(ret == 0)
			break;

		sprintf(buf_write, "client %d: ", ids[fd]);
		send_all(fd, buf_write);
		send_all(fd, msg);
		free(msg);
	}
}

int main(int ac, char **av)
{
	struct sockaddr_in servaddr;
	socklen_t addr_len;
	int client_fd = -1;
	int fd = -1;
	int bytes = 0;

	if(ac != 2)
	{
		deu merda;
		exit(1);
	}

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0)
		fatal_error();
	FD_ZERO(&afds);
	FD_SET(sock_fd, &afds);
	bzero(servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

	if((bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
		fatal_error();

	if (listen(sockfd, 10) < 0)
		fatal_error();

	while(1)
	{
		rfds = afds;
		wfds = afds;
		fd = 0;

		while(fd <= max_fd)
		{
			if(FD_ISSET(fd, &rfds))
			{
				if(sock_fd == fd)
				{
					addr_len = sizeof(servaddr);
					client_fd = accept(sock_fd, (struct sockaddr *)&servaddr, &addr_len);
					if(client_fd >= 0)
						add_client(client_fd);
				}
				else{
						bytes = recv(fd, buf_read, 1000, 0);
						if(bytes <= 0)
							remove_user(fd);
						else
						{
							buf_read[bytes] = 0;
							msgs[fd] = str_join(msgs[fd], buf_read);

							if(msgs[fd] == 0)
								fatal_error();
							handle_msg(buf_read);
						}
				}
			}
			fd++;
		}

		
	}
}