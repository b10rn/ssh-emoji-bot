#include <stdio.h>
#include <libssh/libssh.h>
#include <stdlib.h>
#include <string.h>

#define BOT_USER "emoji-bot"

#define NUM_EMOJIS 9

char *emojis[NUM_EMOJIS];
char *emoji_names[NUM_EMOJIS];

#define EMOJI_SLEEPING 0
#define EMOJI_SMILE 1
#define EMOJI_OPEN_MOUTH 2
#define EMOJI_DIZZY 3
#define EMOJI_KISS 4
#define EMOJI_RELIEVED 5
#define EMOJI_SHRUG 6
#define EMOJI_ASTONISHED 7
#define EMOJI_UNAMUSED 8

void init_emojis()
{
	emojis[EMOJI_SLEEPING] =   "( ᴗ˳ᴗ) zZ";
	emoji_names[EMOJI_SLEEPING] = ":sleeping:";

	emojis[EMOJI_SMILE] =      "(, ^ .^)";
	emoji_names[EMOJI_SMILE] =      ":smile:";

	emojis[EMOJI_OPEN_MOUTH] = "(, ^ o^)";
	emoji_names[EMOJI_OPEN_MOUTH] = ":open_mouth:";

	emojis[EMOJI_DIZZY] =      "(, x .x)";
	emoji_names[EMOJI_DIZZY] =      ":dizzy:";

	emojis[EMOJI_KISS] =      "( ˘³˘)";
	emoji_names[EMOJI_KISS] =      ":kiss:";

	emojis[EMOJI_RELIEVED] =      "( ˘ω˘)";
	emoji_names[EMOJI_RELIEVED] =      ":relieved:";

	emojis[EMOJI_SHRUG] =      "┐(￣ε￣)┌";
	emoji_names[EMOJI_SHRUG] =      ":shrug:";

	emojis[EMOJI_ASTONISHED] =      "( ◎ ˳◎)";
	emoji_names[EMOJI_ASTONISHED] =      ":astonished:";

	emojis[EMOJI_UNAMUSED] =      "( ¬､¬)";
	emoji_names[EMOJI_UNAMUSED] =      ":unamused:";
}

void usage()
{
	printf("usage: ssh-emoji-bot [host] [port]\n");
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Error: Wrong number of arguments\n");
		usage();
		exit(EXIT_FAILURE);
	}

	ssh_session sess = ssh_new();
	if (sess == NULL) {
		fprintf(stderr, "Error: Failed setting up SSH session.\n");
		exit(EXIT_FAILURE);
	}

	init_emojis();

	char *host = argv[1];
	char *user = BOT_USER;
	int port = atoi(argv[2]);

	ssh_options_set(sess, SSH_OPTIONS_HOST, host);
	//ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(sess, SSH_OPTIONS_PORT, &port);
	ssh_options_set(sess, SSH_OPTIONS_USER, user);

	int conn = ssh_connect(sess);
	if (conn != SSH_OK) {
		fprintf(stderr, "Error: Failed connecting to %s:%d\n", host, port);
		exit(EXIT_FAILURE);
	}


	enum ssh_known_hosts_e state;
	state = ssh_session_is_known_server(sess);

	switch (state) {
		case SSH_KNOWN_HOSTS_OK:
			printf("ok\n");
			break;
		case SSH_KNOWN_HOSTS_CHANGED:
			printf("changed\n");
			break;
		case SSH_KNOWN_HOSTS_OTHER:
			printf("other\n");
			break;
		case SSH_KNOWN_HOSTS_NOT_FOUND:
			printf("not found\n");
			break;
		case SSH_KNOWN_HOSTS_UNKNOWN:
			printf("unknown\n");
		case SSH_KNOWN_HOSTS_ERROR:
			printf("error\n");
	}

	int authret = ssh_userauth_autopubkey(sess, NULL);
	if (authret != SSH_AUTH_SUCCESS) {
		fprintf(stderr, "Error: Failed authentication\n");
		exit(EXIT_FAILURE);
	}

	ssh_channel channel;
	channel = ssh_channel_new(sess);
	if (channel == NULL) {
		fprintf(stderr, "Error: Channel failed\n");
		exit(EXIT_FAILURE);
	}

	int rc;
	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK) {
		fprintf(stderr, "Error: Failed opening session\n");
		exit(EXIT_FAILURE);
		//TODO: disconnect clean up etc.
	}

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) {
		fprintf(stderr, "Error: Failed requesting shell\n");
		exit(EXIT_FAILURE);
	}

	int nbytes;
	char buffer[512];

	strcpy(buffer, "(, ^ .^)` helo boyz!\r\n");
	size_t msg_len = strlen(buffer);
	nbytes = ssh_channel_write(channel, buffer, msg_len);

	int write_emoji = -1;
	while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer) - 1, 0);
		if (nbytes > 0) {
			buffer[nbytes] = '\0';
			printf("read message '%s'\n", buffer);
			for (int i = 0; i < NUM_EMOJIS; i++) {
				if (strstr(buffer, emoji_names[i])) {
					write_emoji = i;
					break;
				}
			}
		}

		if (write_emoji >= 0) {
			printf("writing emoji %d\n", write_emoji);
			snprintf(buffer, strlen(emojis[write_emoji]) + 3, "%s\r\n", emojis[write_emoji]);
			printf("writing message '%s'\n", buffer);
			size_t msg_len = strlen(buffer);
			nbytes = ssh_channel_write(channel, buffer, msg_len);
			if (nbytes != msg_len) {
				fprintf(stderr, "Error: Failed write\n");
				exit(EXIT_FAILURE);
			}
			write_emoji = -1;
		}
	}

	ssh_disconnect(sess);
	ssh_free(sess);
	return EXIT_SUCCESS;
}
