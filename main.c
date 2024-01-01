#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define MYINET_PORTMAX 65535
#define MAX_LEN 1024
#define DEFAULT_PORT "1024"
#define DEFAULT_MSG "test message"


const char *argp_program_version = "sockyc v1.00";
const char *argp_program_bug_address = "redacted@fake.org";
static char doc[] = "Socket client CLI\n\n"
					"Default message, if MESSAGE is not specified, is \"" DEFAULT_MSG "\"";
static char args_doc[] = "HOST [MESSAGE]";
static struct argp_option options[] = {
	// name,  key, arg, flags, docstring, group
	{ "port", 'p', "PORT", 0, "destination port on server (default " DEFAULT_PORT ")", 0},
	{ "verbose", 'v', 0, 0, "log feedback to stdout (off by default)", 0 },
	{ 0 },
};

struct arguments {
	char *host;
	char *message;
	char *port;
	bool verbose;
};

bool is_valid_port(char *pport) {
	long int portvalue;

	for (int i = 0; pport[i] != 0; i++) {
		if (pport[i] < '0' || pport[i] > '9') {
			return false;
		}
	}

	portvalue = strtol(pport, 0, 0);
	if (portvalue <= 0 || portvalue > MYINET_PORTMAX) {
		return false;
	}

	return true;
}


error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch (key) {
		case 'p':
			if (is_valid_port(arg)) {
				arguments->port = arg;
			}
			else {
				argp_usage(state);
			}
			break;
		case 'v':
			arguments->verbose = true;
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num == 0) {
				arguments->host = arg;
			}
			else if (state->arg_num == 1) {
				arguments->message = arg;
			}
			else {
				// too many args!
				argp_usage(state);
			}
			break;
		case ARGP_KEY_END:
			if (state->arg_num < 1) {
				// not enough args
				argp_usage(state);
			}
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}
	return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int send_message(char *ipaddr, char *port, char* message, bool verbose);

int main(int argc, char *argv[]) {
	struct arguments arguments;

	/* defaults */
	arguments.message = DEFAULT_MSG;
	arguments.port = DEFAULT_PORT;
	arguments.verbose = false;

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	send_message(arguments.host, arguments.port, arguments.message, arguments.verbose);


	exit (0);
}

int send_message(char *ipaddr, char *port, char* message, bool verbose) {
	char EOT = 4;

	int sfd, s;
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	struct sockaddr_in *insockaddr;
	char *resd_v4addr;



	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_TCP;
	
	s = getaddrinfo(ipaddr, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// getaddrinfo returns result as a linked list of addrinfo
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		// try each socket until we can successfully connect(2)
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

		if (sfd == -1) {
			continue;
		}

		// rp->*ai_addr is struct sockaddr
		insockaddr = (struct sockaddr_in *)rp->ai_addr;

		resd_v4addr = inet_ntoa(insockaddr->sin_addr);

		if (verbose) {
			printf("Connecting to %s\n", resd_v4addr);
		}



		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break;
		}

		close(sfd);
	}

	freeaddrinfo(result);

	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	char buffer[MAX_LEN];

	long unsigned msglen = strlen(message);

	if (msglen + 1 > MAX_LEN) {
		printf("Data to transmit too large, aborting (max size %i bytes)\n", MAX_LEN);
		close(sfd);
		exit(1);
	}


	memcpy(buffer, message, msglen+1);
	buffer[msglen] = EOT;

	ssize_t sendresult;

	sendresult = send(sfd, buffer, msglen + 1, 0);

	if (verbose) {
		printf("successfully sent %li bytes!\n", sendresult);
	}
	close(sfd);



	return 0;
}

