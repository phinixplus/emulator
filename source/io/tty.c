#include "tty.h"

#include <assert.h>
#include <stdio.h>
#include <stdatomic.h>

#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include "io/portdefs.h"

static struct {
	atomic_bool is_init;
	atomic_uint max_clients;
	cpu_t *irq_cpu;

	pthread_mutex_t mutex;
	pthread_t server_thread;
	struct pollfd server_descr;

	uint32_t dev_select;
	uint32_t req_bitmap_save;
	uint32_t req_bitmap_sample;

	struct client_state {
		struct pollfd descriptor;
		io_fifo_t cpubound_fifo;
		io_fifo_t ttybound_fifo;
		uint16_t cpubound_fifo_threshold;
		uint16_t ttybound_fifo_threshold;
		uint8_t displayed_id;
		uint8_t reserved; // Might use this later
		enum read_state {
			READ_START = 0,
			READ_START_CR,
			READ_IAC,  READ_CMD,
			READ_SUB1, READ_SUB2
		} read_state : 8;
		enum write_state {
			WRITE_START = 0,
			WRITE_START_CR,
		} write_state : 8;
	} clients[TTY_MAX_CLIENTS];
} state = {0};

static const uint8_t telnet_preamble[] = {
	255, 251, 0, // IAC WILL ECHO
	255, 251, 1, // IAC WILL BIN
	255, 251, 3  // IAC WILL SGA
};

static unsigned setup_client(
	struct pollfd *server_descr,
	struct client_state *client_pool,
	unsigned max_clients
) {
	assert(poll(server_descr, 1, 0) >= 0);
	if(!(server_descr->revents & POLLIN)) return max_clients;

	int new_client = accept(server_descr->fd, NULL, NULL);
	assert(new_client >= 0);

	unsigned i = 0;
	for(; i < max_clients; i++) {
		if(client_pool[i].descriptor.fd != -1) continue;
		ssize_t size = (ssize_t)(sizeof telnet_preamble);
		if(send(new_client, telnet_preamble, size, 0) != size) break;

		client_pool[i].cpubound_fifo = io_fifo_new();
		client_pool[i].ttybound_fifo = io_fifo_new();
		client_pool[i].descriptor.fd = new_client;
		printf("Connected: TTY%u\n", client_pool[i].displayed_id);
		break;
	}

	if(i == max_clients) {
		assert(close(new_client) == 0);
		printf("Connected: Refused!\n");
	}

	return i;
}

static void close_client(struct client_state *client) {
	assert(close(client->descriptor.fd) == 0);
	client->descriptor.fd = -1;
	io_fifo_del(client->cpubound_fifo);
	io_fifo_del(client->ttybound_fifo);
	printf("Disconnected: TTY%u\n", client->displayed_id);
}

static bool handle_client_read(struct client_state *client) {
	unsigned char raw_buffer[IO_FIFO_SIZE];
	int raw_count = recv(client->descriptor.fd, raw_buffer, IO_FIFO_SIZE, 0);
	if(raw_count == 0 || (raw_count == -1 && errno == ECONNRESET)) return false;
	assert(raw_count > 0);

	unsigned char clean_buffer[IO_FIFO_SIZE];
	unsigned clean_count = 0;
	for(unsigned i = 0; i < (unsigned) raw_count; i++) {
		unsigned char current = raw_buffer[i];
		#define accept(C,S) if(current == C) { client->read_state = S; break; }
		#define reject(C,S) if(current == C) { client->read_state = S; continue; }
		switch(client->read_state) {
			case READ_START:
				accept(0x0D, READ_START_CR)
				reject(0xFF, READ_IAC)
				accept(current, READ_START)
				break;
			case READ_START_CR:
				reject(0x00, READ_START)
				accept(current, READ_START)
				break;
			case READ_IAC:
				accept(0xFF, READ_START)
				reject(0xFA, READ_SUB1)
				reject(0xFB, READ_CMD)
				reject(0xFC, READ_CMD)
				reject(0xFD, READ_CMD)
				reject(0xFE, READ_CMD)
				reject(current, READ_START)
				break;
			case READ_CMD:
				reject(current, READ_START)
				break;
			case READ_SUB1:
				reject(0xFF, READ_SUB2)
				reject(current, READ_SUB1)
				break;
			case READ_SUB2:
				reject(0xF0, READ_START)
				reject(current, READ_SUB1)
				break;
		}
		#undef accept
		#undef reject
		clean_buffer[clean_count++] = current;
	}

	printf("TTY%u: Got %u/%u byte(s)\n",
		client->displayed_id, clean_count, raw_count);
	uint32_t fifo_space = io_fifo_space_free(client->cpubound_fifo);
	if(clean_count > fifo_space) {
		printf("TTY%u: Tossing %d CPU-bound bytes(s)\n",
			client->displayed_id, clean_count - fifo_space);
		clean_count = fifo_space;
	}

	for(unsigned i = 0; i < clean_count; i++) {
		uint32_t data = clean_buffer[i];
		io_fifo_write(client->cpubound_fifo, &data);
	}

	return true;
}

static bool handle_client_write(struct client_state *client) {
	uint32_t clean_count = io_fifo_space_used(client->ttybound_fifo);
	if(clean_count == 0) return true;

	// Twice the IO_FIFO_SIZE is fool-proof because, only for CR and IAC,
	// there will need to be another character afterwards and the rest are 1-1.
	// So it is enough to allocate double for the worst case of all CR/IAC.
	unsigned char raw_buffer[IO_FIFO_SIZE * 2];
	unsigned raw_count = 0;
	for(unsigned i = 0; i < clean_count; i++) {
		uint32_t data; io_fifo_read(client->ttybound_fifo, &data);
		unsigned char current = (unsigned char)(data & 0xFF);
		switch(client->write_state) {
			case WRITE_START:
				if(current == 0x0D) client->write_state = WRITE_START_CR;
				else client->write_state = WRITE_START;
				if(current == 0xFF) raw_buffer[raw_count++] = current;
				raw_buffer[raw_count++] = current;
				break;
			case WRITE_START_CR:
				client->write_state = WRITE_START;
				if(current != 0x0A) raw_buffer[raw_count++] = 0x00;
				raw_buffer[raw_count++] = current;
				break;
		}
	}

	bool success = (send(client->descriptor.fd, raw_buffer, raw_count, 0) == raw_count);
	printf("TTY%u: Sent %u/%u byte(s)\n", client->displayed_id, clean_count, raw_count);
	return success;
}

static uint32_t process_events(void) {
	uint32_t req_bitmap = 0;
	unsigned max_clients = atomic_load(&state.max_clients);
	unsigned new_client = setup_client(&state.server_descr, state.clients, max_clients);
	if(new_client != max_clients) req_bitmap |= MASK(1, new_client);

	for(unsigned i = 0; i < max_clients; i++) {
		struct client_state *client = &state.clients[i];
		if(client->descriptor.fd == -1) continue;

		assert(poll(&client->descriptor, 1, 0) >= 0);
		bool still_open = true;

		if(client->descriptor.revents & POLLIN)
			still_open &= handle_client_read(client);
		uint32_t used = io_fifo_space_used(client->cpubound_fifo);
		if(used > client->cpubound_fifo_threshold)
			req_bitmap |= MASK(1, i);

		if((client->descriptor.revents & POLLOUT) && still_open)
			still_open &= handle_client_write(client);
		uint32_t free = io_fifo_space_free(client->ttybound_fifo);
		if(free > client->ttybound_fifo_threshold)
			req_bitmap |= MASK(1, i);

		if(client->descriptor.revents & POLLHUP || !still_open)
			close_client(client), req_bitmap |= MASK(1, i);
	}
	return req_bitmap;
}

static void *tty_thread(void *dummy) {
	(void) dummy;
	while(atomic_load(&state.is_init)) {
		pthread_mutex_lock(&state.mutex);
		uint32_t req_bitmap = process_events();
		state.req_bitmap_save = req_bitmap;
		pthread_mutex_unlock(&state.mutex);

		if(req_bitmap != 0) ipm_interrupt(state.irq_cpu, IRQ_IO_TTY);
		struct timespec duration = {0, 100 * 1000}; // 100us
		nanosleep(&duration, NULL);
	}
	pthread_exit(NULL);
}

static void ttycon_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	assert(!rw_select);

	unsigned max_clients = atomic_load(&state.max_clients);
	pthread_mutex_lock(&state.mutex);

	state.req_bitmap_sample = state.req_bitmap_save;
	for(unsigned i = 0; i < max_clients; i++) {
		if(state.clients[i].descriptor.fd == -1) continue;
		*rw_data |= 1 << i;
	}

	pthread_mutex_unlock(&state.mutex);
}

static void ttyreq_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	if(rw_select) state.dev_select = EXTRACT(*rw_data, 7, 0);
	else *rw_data = state.req_bitmap_sample;
}

static void ttydata_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	unsigned tty_select = EXTRACT(state.dev_select, 5, 0);
	if(tty_select >= atomic_load(&state.max_clients)) return;
	struct client_state *client = &state.clients[tty_select];

	pthread_mutex_lock(&state.mutex);
	if(client->descriptor.fd == -1) {
		pthread_mutex_unlock(&state.mutex);
		return;
	}

	if(rw_select) io_fifo_write(client->ttybound_fifo, rw_data);
	else if(!io_fifo_read(client->cpubound_fifo, rw_data))
		*rw_data = 0x80000000; // If the FIFO was empty, signal using sign bit

	pthread_mutex_unlock(&state.mutex);
}

static void ttystat_callback(bool rw_select, uint32_t *rw_data, void *context) {
	(void) context;
	unsigned tty_select = EXTRACT(state.dev_select, 5, 0);
	unsigned csr_select = EXTRACT(state.dev_select, 2, 5);
	if(tty_select >= atomic_load(&state.max_clients)) return;
	struct client_state *client = &state.clients[tty_select];

	pthread_mutex_lock(&state.mutex);
	if(client->descriptor.fd == -1) {
		pthread_mutex_unlock(&state.mutex);
		return;
	}

	switch(csr_select) {
		case 0: // Read CPU-bound buffer character amount
			if(rw_select) break;
			*rw_data = io_fifo_space_used(client->cpubound_fifo);
			break;
		case 1: // Read TTY-bound buffer free space
			if(rw_select) break;
			*rw_data = io_fifo_space_free(client->ttybound_fifo);
			break;
		case 2: // Read-Write CPU-bound IRQ threshold
			if(rw_select) client->cpubound_fifo_threshold = *rw_data;
			else *rw_data = client->cpubound_fifo_threshold;
			break;
		case 3: // Read-Write TTY-bound IRQ threshold
			if(rw_select) client->ttybound_fifo_threshold = *rw_data;
			else *rw_data = client->ttybound_fifo_threshold;
			break;
	}

	pthread_mutex_unlock(&state.mutex);
}

bool tty_setup(
	io_t io, cpu_t *irq_cpu,
	uint16_t server_port,
	unsigned max_clients
) {
	if(atomic_load(&state.is_init)) return false;

	state.server_descr.events = POLLIN; // informs of waiting clients
	for(unsigned i = 0; i < TTY_MAX_CLIENTS; i++) {
		state.clients[i].descriptor.fd = -1; // ignored by poll
		state.clients[i].descriptor.events = POLLIN | POLLOUT;
		state.clients[i].displayed_id = i + 1;
		state.clients[i].read_state = READ_START;
		state.clients[i].write_state = WRITE_START;
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
	assert(io_try_attach(io, IO_TTYDATA, ttydata_callback, NULL));
	assert(io_try_attach(io, IO_TTYSTAT, ttystat_callback, NULL));

	pthread_mutex_init(&state.mutex, NULL);
	atomic_store(&state.is_init, true);
	atomic_store(&state.max_clients, max_clients);
	pthread_create(&state.server_thread, NULL, tty_thread, NULL);

	return true;
}

bool tty_close(io_t io) {
	if(!atomic_load(&state.is_init)) return false;
	atomic_store(&state.is_init, false);

	pthread_join(state.server_thread, NULL);
	pthread_mutex_destroy(&state.mutex);

	assert(close(state.server_descr.fd) == 0);
	unsigned max_clients = atomic_load(&state.max_clients);
	for(unsigned i = 0; i < max_clients; i++) {
		if(state.clients[i].descriptor.fd == -1) continue;
		close_client(&state.clients[i]);
	}

	assert(io_try_detach(io, IO_TTYCON,  NULL, NULL));
	assert(io_try_detach(io, IO_TTYREQ,  NULL, NULL));
	assert(io_try_detach(io, IO_TTYDATA, NULL, NULL));
	assert(io_try_detach(io, IO_TTYSTAT, NULL, NULL));

	return true;
}
