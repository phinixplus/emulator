#include "telnet.h"

#include <netinet/ip.h>
#include <sys/socket.h>
#include <assert.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

#include "io/portdefs.h"

typedef int fdesc_t;

static struct {
	fdesc_t client_socket;
	struct pollfd poll_descriptor;
	bool read_was_cr;
	enum {
		START, EXIT,
		IAC, CMD,
		SUB1, SUB2
	} read_state;
} state;

static void telnet_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	if(state.client_socket == -1) {
		if(!rw_select) *rw_data = 0xFFFFFFFF;
		return;
	}

	uint8_t character;
	if(rw_select) {
		ssize_t ret, req;
		character = *rw_data & 0xFF;
		if(character == 0xFF) ret = write(state.client_socket, "\xff\xff", req = 2);
		else if(character == '\r') ret = write(state.client_socket, "\r\0", req = 2);
		else ret = write(state.client_socket, &character, req = 1);
		if(ret != req) exit(EXIT_FAILURE);
		return;
	}

	while(true) {
		poll(&state.poll_descriptor, 1, 0);
		if(!(state.poll_descriptor.revents & POLLIN)) { *rw_data = 0x80000000; return; }
		if(read(state.client_socket, &character, 1) != 1) exit(EXIT_FAILURE);

		// This is what Big Telnet demands you do to merely ignore the negotiation
		// It will definitely not come back to bite me in the ass in a few months...
		switch(state.read_state) {
			case START:
				if(character == 255) state.read_state = IAC;
				else if(state.read_was_cr && character == '\0') state.read_state = START;
				else state.read_was_cr = (character == '\r'), state.read_state = EXIT;
				break;
			case IAC: switch(character) {
				case 241: case 242: case 243: // NOP, DM, BRK
				case 244: case 245: case 246: // IP, AO, AYT
				case 247: case 248: case 249: // EC, EL, GA
					state.read_state = START; break;
				case 250: // SB
					state.read_state = SUB1; break;
				case 251: case 252: // WILL WONT
				case 253: case 254: // DO DONT
					state.read_state = CMD; break;
				default:
					state.read_state = EXIT; break;
			} break;
			case CMD:
				state.read_state = START;
				break;
			case SUB1:
				if(character == 255) state.read_state = SUB2;
				else state.read_state = SUB1;
				break;
			case SUB2:
				if(character == 240) state.read_state = START;
				else state.read_state = SUB1;
				break;
			case EXIT:
				break;
		}

		if(state.read_state == EXIT) {
			state.read_state = START;
			*rw_data = character;
			return;
		}
	}
}

bool telnet_setup(io_t io, uint16_t net_port) {
	struct sockaddr_in descriptor;
	descriptor.sin_family = AF_INET;
	descriptor.sin_addr.s_addr = INADDR_ANY;
	descriptor.sin_port = htons(net_port);

	fdesc_t server_socket;
	if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) goto fail0;
	struct sockaddr *generic_descr = (struct sockaddr *) &descriptor;
	if(bind(server_socket, generic_descr, sizeof descriptor) != 0) goto fail1;
	if(listen(server_socket, 1) != 0) goto fail1;
	if((state.client_socket = accept(server_socket, NULL, NULL)) < 0) goto fail1;

	unsigned char preamble[] = { 255, 251, 0, 255, 251, 1, 255, 251, 3 };
	if(write(state.client_socket, preamble, sizeof preamble) != sizeof preamble) goto fail2;
	if(!io_try_attach(io, IO_TELNET, telnet_callback, NULL)) goto fail2;

	state.poll_descriptor.fd = state.client_socket;
	state.poll_descriptor.events = POLLIN;
	state.read_was_cr = false;
	state.read_state = START;
	
	close(server_socket);
	return true;

	fail2: close(state.client_socket);
	fail1: close(server_socket);
	fail0: return false;
}

void telnet_close(io_t io) {
	if(state.client_socket != -1) {
		close(state.client_socket);
		state.client_socket = -1;
		assert(io_try_detach(io, IO_TELNET, NULL, NULL));
	}
}
