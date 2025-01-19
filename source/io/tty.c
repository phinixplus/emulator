#include "tty.h"

#include <stdio.h>

#include <assert.h>
#include <pthread.h>

#include <poll.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>

static struct {
	volatile bool init;
	pthread_t server_thread;
	struct pollfd server_descr;
	struct pollfd client_descrs[TTY_MAX_CLIENTS];
} state = {0};

static void *tty_thread(void *dummy) {
	(void) dummy;
	while(state.init) {
		assert(poll(&state.server_descr, 1, 0) >= 0);
		if(state.server_descr.revents & POLLIN) {
			int new_client = accept(state.server_descr.fd, NULL, NULL);
			assert(new_client >= 0);

			bool found = false;
			for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
				if(state.client_descrs[i].fd != -1) continue;
				state.client_descrs[i].fd = new_client;
				found = true;
				printf("Connected: TTY%d\n", i + 1);
				break;
			}

			if(!found) {
				assert(close(new_client) == 0);
				printf("Connected: Refused!\n");
			}
		}

		assert(poll(state.client_descrs, TTY_MAX_CLIENTS, 0) >= 0);
		for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
			if(state.client_descrs[i].fd == -1) continue;
			if(!(state.client_descrs[i].revents & POLLIN)) continue;

			// -- Temporary TCP Echo -- //
			char buffer[16];
			ssize_t count = recv(state.client_descrs[i].fd, buffer, 16 , 0);
			assert(count >= 0);

			if(count == 0) {
				assert(close(state.client_descrs[i].fd) == 0);
				state.client_descrs[i].fd = -1;
				printf("Disconnected: TTY%d\n", i + 1);
				continue;
			}

			assert(send(state.client_descrs[i].fd, buffer, count, 0) >= count);
			printf("TTY%d: %ld byte(s)\n", i + 1, count);
			// ------------------------ //
		}

		struct timespec duration = {0, 100 * 1000}; // 100us
		nanosleep(&duration, NULL);
	}
	pthread_exit(NULL);
}

bool tty_setup(io_t io, uint16_t server_port) {
	(void) io;
	if(state.init) return false;

	state.server_descr.events = POLLIN;
	for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
		state.client_descrs[i].fd = -1; // ignored by poll
		state.client_descrs[i].events = POLLIN;
	}

	struct sockaddr_in descriptor;
	descriptor.sin_family = AF_INET;
	descriptor.sin_addr.s_addr = INADDR_ANY;
	descriptor.sin_port = htons(server_port);

	assert((state.server_descr.fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
	struct sockaddr *generic_descr = (struct sockaddr *) &descriptor;
	assert(bind(state.server_descr.fd, generic_descr, sizeof descriptor) == 0);
	assert(listen(state.server_descr.fd, 1) == 0);

	pthread_create(&state.server_thread, NULL, tty_thread, NULL);

	state.init = true;
	return true;
}

bool tty_close(io_t io) {
	(void) io;
	if(!state.init) return false;
	state.init = false;

	pthread_join(state.server_thread, NULL);
	assert(close(state.server_descr.fd) == 0);
	for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
		if(state.client_descrs[i].fd == -1) continue;
		assert(close(state.client_descrs[i].fd) == 0);
	}

	return true;
}
