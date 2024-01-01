#include <stdio.h>
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
#define PROG_NAME "sockyc"
#define PROG_VERSION "sockyc v1.00"
#define PROG_BUGADDR "redacted@fake.org"

void showversion() {
	puts(PROG_VERSION);
	return;
}

void showusage(char **argv) {
	char *progname;
	if (argv != NULL) {
		progname = argv[0];
	}
	else {
		progname = PROG_NAME;
	}

	printf("Usage: %s [OPTION...] HOST [MESSAGE]\n", progname);
	return;
}

void showhelp(char **argv) {
	showusage(argv);
	
	const char helpintro[] = "TCP socket message client CLI\n\n"
							  "Default message, if MESSAGE is not specified, is \"" DEFAULT_MSG "\"\n";


	const char *options[] = {
		"  -p PORT       Destination port on server (default " DEFAULT_PORT ")",
		"  -v            Log feedback to stdout (off by default)",
		"  -?            Show this help and exit",
		"  -V            Show program version and exit"
	};

	const char helpoutro[] = "Report bugs to " PROG_BUGADDR ".";

	printf("%s\n", helpintro);
	for (int i=0; i < sizeof(options) / sizeof(*options); i++) {
		printf("%s\n", options[i]);
	}
	printf("\n%s\n", helpoutro);
	return;
}

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

int parse_getopt(int argc, char **argv, struct arguments *arguments) {
	int c;

	while ((c = getopt(argc, argv, "p:vV")) != -1) {
		switch (c) {
			case 'p':
				if (is_valid_port(optarg)) {
					arguments->port = optarg;
				}
				else {
					fprintf(stderr, "Invalid port\n");
					exit(1);
				}
				break;
			case 'v':
				arguments->verbose = true;
				break;
			case '?':
				showhelp(argv);
				exit(0);
			case 'V':
				showversion();
				exit(0);
			default:
				showusage(argv);
				exit(1);
		}
	}
}

int send_message(char *ipaddr, char *port, char* message, bool verbose);

int main(int argc, char *argv[]) {
	// I don't think i've alloc'd memory for the strings here...
	// does getopt allocate memory?
	// or does the value of the optarg pointer change; we end up storing a reference
	// to argv (that would make sense?)
	// Confirmed by the docs!
	struct arguments arguments;

	/* defaults */
	arguments.message = DEFAULT_MSG;
	arguments.port = DEFAULT_PORT;
	arguments.verbose = false;

	parse_getopt(argc, argv, &arguments);

	// positional args
	if (optind >= argc || optind + 2 < argc) {
		showusage(argv);
		exit(1);
	}
	arguments.host = argv[optind];
	if (optind + 1 < argc) {
		arguments.message = argv[optind+1];
	}

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
	char buffer[MAX_LEN];
	size_t bufsize;
	ssize_t sendresult;

	long unsigned msglen = strlen(message);

	bufsize = msglen + 1;

	if (bufsize > MAX_LEN) {
		fprintf(stderr, "Data to transmit too large, aborting (max size %i bytes)\n", MAX_LEN);
		exit(1);
	}

	memcpy(buffer, message, bufsize);
	buffer[msglen] = EOT;

	// could use a des'd initzer
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


	sendresult = send(sfd, buffer, bufsize, 0);

	if (verbose) {
		printf("successfully sent %li bytes!\n", sendresult);
	}
	close(sfd);



	return 0;
}

