#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>

int		g_id = 0;
int		ids[65536];
char	*msgs[65536];
int		max_fd = 0;
int		sock_fd = 0;

fd_set	rfds, wfds, afds;
char	buf_read[1001], buf_write[1001];

int	extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int		i;

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

char	*str_join(char *buf, char *add)
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

void	fatal_error(void)
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	send_all(int skip_fd, char *str)
{
	int	fd;

	fd = 0;
	while (fd <= max_fd)
	{
		if (FD_ISSET(fd, &afds) && FD_ISSET(fd, &wfds) && fd != skip_fd && fd != sock_fd)
			send(fd, str, strlen(str), 0);
		fd++;
	}
}

void	add_client(int fd)
{
	FD_SET(fd, &afds);
	ids[fd] = g_id++;
	msgs[fd] = NULL;
	if (fd > max_fd)
		max_fd = fd;
	sprintf(buf_write, "server: client %d just arrived\n", ids[fd]);
	send_all(fd, buf_write);
}

void	rm_client(int fd)
{
	sprintf(buf_write, "server: client %d just left\n", ids[fd]);
	send_all(fd, buf_write);
	free(msgs[fd]);
	msgs[fd] = NULL;
	FD_CLR(fd, &afds);
	close(fd);
}

void	handle_msg(int fd)
{
	char	*msg;
	int		ret;

	while (1)
	{
		ret = extract_message(&(msgs[fd]), &msg);
		if (ret < 0)
			fatal_error();
		if (ret == 0)
			break ;
		sprintf(buf_write, "client %d: ", ids[fd]);
		send_all(fd, buf_write);
		send_all(fd, msg);
		free(msg);
	}
}

int	main(int ac, char **av)
{
	struct sockaddr_in	servaddr;
	int					client_fd;
	socklen_t			addr_len;
	int					bytes;
	int					fd;

	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
		fatal_error();
	max_fd = sock_fd;
	FD_ZERO(&afds);
	FD_SET(sock_fd, &afds);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1]));
	if (bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		fatal_error();
	if (listen(sock_fd, SOMAXCONN) < 0)
		fatal_error();
	while (1)
	{
		rfds = afds;
		wfds = afds;
		if (select(max_fd + 1, &rfds, &wfds, NULL, NULL) < 0)
			fatal_error();
		fd = 0;
		while (fd <= max_fd)
		{
			if (FD_ISSET(fd, &rfds))
			{
				if (fd == sock_fd)
				{
					addr_len = sizeof(servaddr);
					client_fd = accept(sock_fd, (struct sockaddr *)&servaddr, &addr_len);
					if (client_fd >= 0)
						add_client(client_fd);
				}
				else
				{
					bytes = recv(fd, buf_read, 1000, 0);
					if (bytes <= 0)
						rm_client(fd);
					else
					{
						buf_read[bytes] = 0;
						msgs[fd] = str_join(msgs[fd], buf_read);
						if (msgs[fd] == 0)
							fatal_error();
						handle_msg(fd);
					}
				}
			}
			fd++;
		}
	}
	return (0);
}
