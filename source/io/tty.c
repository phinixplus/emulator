#include "tty.h"

#include <stdio.h>

#include <assert.h>
#include <pthread.h>

#include <poll.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "io/portdefs.h"

static struct {
	volatile bool init;
	cpu_t *irq_cpu;
	unsigned dev_select;

	pthread_mutex_t mutex;
	pthread_t server_thread;
	struct pollfd server_descr;

	struct pollfd client_descrs[TTY_MAX_CLIENTS];
	io_fifo_t cpubound_fifos[TTY_MAX_CLIENTS];
	io_fifo_t ttybound_fifos[TTY_MAX_CLIENTS];
} state = {0};

static void *tty_thread(void *dummy) {
	(void) dummy;
	while(state.init) {
		pthread_mutex_lock(&state.mutex);
		assert(poll(&state.server_descr, 1, 0) >= 0);
		if(state.server_descr.revents & POLLIN) {
			int new_client = accept(state.server_descr.fd, NULL, NULL);
			assert(new_client >= 0);

			bool found = false;
			for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
				if(state.client_descrs[i].fd != -1) continue;
				state.cpubound_fifos[i] = io_fifo_new();
				state.ttybound_fifos[i] = io_fifo_new();
				state.client_descrs[i].fd = new_client;
				ipm_interrupt(state.irq_cpu, 2);
				printf("Connected: TTY%d\n", i + 1);
				found = true;
				break;
			}

			if(!found) {
				assert(close(new_client) == 0);
				printf("Connected: Refused!\n");
			}
		}

		bool send_irq = false;
		assert(poll(state.client_descrs, TTY_MAX_CLIENTS, 0) >= 0);
		for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
			if(state.client_descrs[i].fd == -1) continue;
			if(!(state.client_descrs[i].revents & POLLIN)) continue;

			unsigned char buffer[TTY_BUFFER_SIZE];
			ssize_t count = recv(
				state.client_descrs[i].fd,
				buffer, TTY_BUFFER_SIZE , 0
			);
			if(count == 0) {
				assert(close(state.client_descrs[i].fd) == 0);
				state.client_descrs[i].fd = -1;
				io_fifo_del(state.cpubound_fifos[i]);
				io_fifo_del(state.ttybound_fifos[i]);
				ipm_interrupt(state.irq_cpu, 2);
				printf("Disconnected: TTY%d\n", i + 1);
				continue;
			} else assert(count > 0);
			printf("TTY%d: Got %ld byte(s)\n", i + 1, count);

			uint32_t fifo_space = io_fifo_space(state.cpubound_fifos[i]);
			if(count > fifo_space) {
				ssize_t difference = count - fifo_space;
				printf("TTY%d: Tossing %ld CPU-bound bytes(s)\n", i + 1, difference);
				count = fifo_space;
			}
			for(int j = 0; j < count; j++) {
				uint32_t data = buffer[j];
				io_fifo_write(state.cpubound_fifos[i], &data);
			}
			send_irq = true;
		}

		for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
			if(state.client_descrs[i].fd == -1) continue;
			uint32_t count = (1 << IO_FIFO_SIZE_BITS);
			count -= io_fifo_space(state.ttybound_fifos[i]);
			if(count == 0) continue;

			unsigned char buffer[TTY_BUFFER_SIZE];
			for(unsigned j = 0; j < count; j++) {
				uint32_t data;
				io_fifo_read(state.ttybound_fifos[i], &data);
				buffer[j] = (unsigned char)(data & 0xFF);
			}

			assert(send(state.client_descrs[i].fd, buffer, count, 0) == count);
			printf("TTY%d: Sent %u byte(s)\n", i + 1, count);
		}

		if(send_irq) ipm_interrupt(state.irq_cpu, 2);

		pthread_mutex_unlock(&state.mutex);
		struct timespec duration = {0, 100 * 1000}; // 100us
		nanosleep(&duration, NULL);
	}
	pthread_exit(NULL);
}

static void ttycon_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	assert(!rw_select);
	pthread_mutex_lock(&state.mutex);
	for(int i = 0; i < TTY_MAX_CLIENTS; i++)
		if(state.client_descrs[i].fd != -1)
			*rw_data |= 1 << i;
	pthread_mutex_unlock(&state.mutex);
}

static void ttyreq_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	assert(!rw_select);
	pthread_mutex_lock(&state.mutex);
	for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
		if(state.client_descrs[i].fd == -1) continue;
		if(io_fifo_space(state.cpubound_fifos[i]) < (1 << IO_FIFO_SIZE_BITS))
			*rw_data |= 1 << i;
	}
	pthread_mutex_unlock(&state.mutex);
}

static void ttysel_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	if(rw_select) state.dev_select = *rw_data & ((1 << 6) - 1);
	else *rw_data = state.dev_select;
}

static void ttydata_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	unsigned tty_select = state.dev_select & ((1 << 5) - 1);
	if(tty_select >= TTY_MAX_CLIENTS) return;
	pthread_mutex_lock(&state.mutex);
	if(state.client_descrs[tty_select].fd != -1) {
		putchar(*rw_data);
		if(rw_select) io_fifo_write(state.ttybound_fifos[tty_select], rw_data);
		else if(!io_fifo_read(state.cpubound_fifos[tty_select], rw_data))
			*rw_data = 0x80000000; // If the FIFO was empty, signal using sign bit
	}
	pthread_mutex_unlock(&state.mutex);
}

static void ttystat_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	assert(!rw_select);
	unsigned tty_select = state.dev_select & ((1 << 5) - 1);
	bool buffer_select = ((state.dev_select & (1 << 5)) != 0);
	if(tty_select >= TTY_MAX_CLIENTS) return;
	pthread_mutex_lock(&state.mutex);
	if(state.client_descrs[tty_select].fd != -1) {
		if(buffer_select) *rw_data = io_fifo_space(state.ttybound_fifos[tty_select]);
		else *rw_data = io_fifo_space(state.cpubound_fifos[tty_select]);
	}
	pthread_mutex_unlock(&state.mutex);
}

bool tty_setup(io_t io, cpu_t *irq_cpu, uint16_t server_port) {
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
	
	int option_value = 1;
	assert((state.server_descr.fd = socket(AF_INET, SOCK_STREAM, 0)) >= 0);
	assert(setsockopt(state.server_descr.fd, SOL_SOCKET,
		SO_REUSEADDR, &option_value, sizeof option_value) == 0);
	struct sockaddr *generic_descr = (struct sockaddr *) &descriptor;
	assert(bind(state.server_descr.fd, generic_descr, sizeof descriptor) == 0);
	assert(listen(state.server_descr.fd, 1) == 0);

	state.irq_cpu = irq_cpu;

	assert(io_try_attach(io, IO_TTYCON,  ttycon_callback,  NULL));
	assert(io_try_attach(io, IO_TTYREQ,  ttyreq_callback,  NULL));
	assert(io_try_attach(io, IO_TTYSEL,  ttysel_callback,  NULL));
	assert(io_try_attach(io, IO_TTYDATA, ttydata_callback, NULL));
	assert(io_try_attach(io, IO_TTYSTAT, ttystat_callback, NULL));

	pthread_mutex_init(&state.mutex, NULL);
	pthread_create(&state.server_thread, NULL, tty_thread, NULL);

	state.init = true;
	return true;
}

bool tty_close(io_t io) {
	(void) io;
	if(!state.init) return false;
	state.init = false;

	pthread_join(state.server_thread, NULL);
	pthread_mutex_destroy(&state.mutex);

	assert(close(state.server_descr.fd) == 0);
	for(int i = 0; i < TTY_MAX_CLIENTS; i++) {
		if(state.client_descrs[i].fd == -1) continue;
		assert(close(state.client_descrs[i].fd) == 0);
		io_fifo_del(state.cpubound_fifos[i]);
		io_fifo_del(state.ttybound_fifos[i]);
		printf("Disconnected: TTY%d\n", i + 1);
	}

	assert(io_try_detach(io, IO_TTYCON,  NULL, NULL));
	assert(io_try_detach(io, IO_TTYREQ,  NULL, NULL));
	assert(io_try_detach(io, IO_TTYSEL,  NULL, NULL));
	assert(io_try_detach(io, IO_TTYDATA, NULL, NULL));
	assert(io_try_detach(io, IO_TTYSTAT, NULL, NULL));

	return true;
}
