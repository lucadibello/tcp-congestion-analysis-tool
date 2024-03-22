#include <unistd.h>
#include <sys/socket.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <libgen.h>
#include <poll.h>

#define BACKLOG 128
#define BUFSIZE 1024
#define DEFAULT_PORT 5000

static struct option cli_options[] = {
		{"port", required_argument, 0, 'p'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0}};

void print_help(FILE *fp, const char *progname)
{
	size_t len = strlen(progname);
	char path[len + 1];
	strcpy(path, progname);
	fprintf(fp, "Usage: %s [options]\n\n", basename(path));

	fprintf(fp, "-p <port>, --port <port>\n");
	fprintf(fp, "                  The port to listen on (default: %u)\n",
					DEFAULT_PORT);
	fprintf(fp, "-h, --help\n");
	fprintf(fp, "                  Display this help and exit\n");
}

void err_quit(const char *msg)
{
	if (errno != 0)
		perror(msg);
	else
		fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

int server_socket(uint16_t port)
{
	struct sockaddr_in addr;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		err_quit("Failed to create a socket");

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		close(sockfd);
		err_quit("Failed to bind the socket");
	}

	if (listen(sockfd, BACKLOG) < 0)
	{
		close(sockfd);
		err_quit("Failed to make the socket listening");
	}

	return sockfd;
}

void handle_connections(int sockfd)
{
	ssize_t n;
	struct pollfd fds[FOPEN_MAX];
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	for (unsigned int i = 1; i < FOPEN_MAX; ++i)
		fds[i].fd = -1;
	unsigned int max = 0;
	char buf[BUFSIZE];

	while (1)
	{
		int nready = poll(fds, max + 1, -1);

		// Read data from the connected clients
		for (unsigned int i = 1; i <= max; ++i)
		{
			if (fds[i].fd < 0)
				continue;

			if (fds[i].revents & (POLLIN | POLLERR))
			{
				if ((n = read(fds[i].fd, buf, BUFSIZE)) < 0)
				{
					if (errno == ECONNRESET)
					{
						close(fds[i].fd);
						fds[i].fd = -1;
					}
					else
					{
						for (unsigned int j = 0; j <= max; ++j)
							if (fds[j].fd >= 0)
								close(fds[j].fd);
						err_quit("Failed reading from client");
					}
				}
				else if (n == 0)
				{
					close(fds[i].fd);
					fds[i].fd = -1;
				}
				if (--nready <= 0)
					break;
			}
		}

		// Accept new connecting clients
		if (fds[0].revents & POLLIN)
		{
			int client = accept(sockfd, NULL, NULL);
			unsigned int i = 1;
			for (i = 1; i < FOPEN_MAX; ++i)
				if (fds[i].fd < 0)
				{
					fds[i].fd = client;
					break;
				}
			if (i == FOPEN_MAX)
			{
				for (unsigned int j = 0; j <= max; ++j)
					if (fds[j].fd >= 0)
						close(fds[j].fd);
				err_quit("Too many files open");
			}
			fds[i].events = POLLIN;
			shutdown(client, SHUT_WR);
			if (i > max)
				max = i;
			if (--nready <= 0)
				continue;
		}
	}
}

int main(int argc, char *argv[])
{
	uint16_t port = DEFAULT_PORT;

	int opt, opt_index;
	while (optind < argc)
	{
		if ((opt =
						 getopt_long(argc, argv, "p:t:c:h", cli_options,
												 &opt_index)) != -1)
		{
			switch (opt)
			{
			case 'p':
			{
				long val = strtol(optarg, NULL, 10);

				if (errno || val <= 0 || val > UINT16_MAX)
					err_quit("Invalid port number provided");
				port = (uint16_t)val;
				break;
			}
			case 'h':
				print_help(stdout, argv[0]);
				return EXIT_SUCCESS;
			default:
				print_help(stderr, argv[0]);
				return EXIT_FAILURE;
			}
		}
		else
		{
			print_help(stderr, argv[0]);
			return EXIT_FAILURE;
		}
	}

	int sockfd = server_socket(port);
	handle_connections(sockfd);
}
