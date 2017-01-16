/*
 * transfer.c
 *
 *  Created on: 9 Jun 2014
 *      Author: nils
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 bkinman, Nils Roos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <pthread.h>
//#include <rp_scope.h>

#include "options.h"
#include "scope.h"
#include "transfer.h"

#include <stdlib.h>
#include <string.h>

/******************************************************************************
 * Defines
 ******************************************************************************/
#define BLOCK_SIZE  16384
#define BUFFER_SIZE 16 * BLOCK_SIZE

#define min(x, y) (((x) < (y)) ? (x) : (y))
/* note: the circular buffer macros may evaluate each of their arguments once, more
 *       than once or not at all. */
#define CIRCULAR(arg, size) \
	((arg) % (size))
#define CIRCULAR_ADD(arg1, arg2, size) \
	(((arg1) + (arg2)) % (size))
#define CIRCULAR_SUB(arg1, arg2, size) \
	((arg1) >= (arg2) ? (arg1) - (arg2) : (size) + (arg1) - (arg2))
#define CIRCULAR_DIST(argfrom, argto, size) \
	CIRCULAR_SUB((argto), (argfrom), (size))
#define CIRCULARSRC_MEMCPY(dst, src_base, src_offs, src_size, length) \
	do { \
		if ((src_offs) + (length) <= (src_size)) { \
			memcpy((dst), (void *)(src_base) + (src_offs), (length)); \
		} else { \
			unsigned int __len1 = (src_size) - (src_offs); \
			memcpy((dst), (void *)(src_base) + (src_offs), __len1); \
			memcpy((void *)(dst) + __len1, (src_base), (length) - __len1); \
		} \
	} while (0)
#define CIRCULAR_MEMCPY(dst_base, dst_offs, dst_size, src_base, src_offs, src_size, length) \
	do { \
		if ((src_offs) + (length) <= (src_size)) { \
			if ((dst_offs) + (length) <= (dst_size)) { \
				memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), (length)); \
			} else { \
				unsigned int __len = (dst_size) - (dst_offs); \
				memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), __len); \
				memcpy((dst_base), (void *)(src_base) + ((src_offs) + __len), (length) - __len); \
			} \
		} else { \
			unsigned int __len1 = (src_size) - (src_offs); \
			if ((dst_offs) + (length) <= (dst_size)) { \
				memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), __len1); \
				memcpy((void *)(dst_base) + ((dst_offs) + __len1), (src_base), (length) - __len1); \
			} else { \
				unsigned int __len2 = (dst_size) - (dst_offs); \
				if (__len1 == __len2) { \
					memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), __len1); \
					memcpy((dst_base), (src_base), (length) - __len1); \
				} else if (__len1 < __len2) { \
					memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), __len1); \
					memcpy((void *)(dst_base) + ((dst_offs) + __len1), (src_base), __len2 - __len1); \
					memcpy((dst_base), (void *)(src_base) + (__len2 - __len1), (length) - __len2); \
				} else { \
					memcpy((void *)(dst_base) + (dst_offs), (void *)(src_base) + (src_offs), __len2); \
					memcpy((dst_base), (void *)(src_base) + ((src_offs) + __len2), __len1 - __len2); \
					memcpy((void *)(dst_base) + (__len1 - __len2), (src_base), (length) - __len1); \
				} \
			} \
		} \
	} while (0)

/******************************************************************************
 * Typedefs
 ******************************************************************************/
struct queue {
	pthread_mutex_t mutex;
	pthread_t       sender;
	int             started;
	unsigned int    write_pos;
	unsigned int    read_pos;
	uint8_t         *buf;
	int             sock_fd;
	FILE            *file_fd;
};

/******************************************************************************
 * static function prototypes
 ******************************************************************************/
static void int_handler(int sig);
static int setup_threads(struct handles *handles);
static void teardown_threads(void);
static u_int64_t transfer_readwrite(struct scope_parameter *param,
                                    option_fields_t *options,
                                    struct handles *handles);
static u_int64_t transfer_buf_mmap(struct scope_parameter *param,
                                   option_fields_t *options,
                                   struct handles *handles);
static u_int64_t transfer_buf_mmap_dual(struct scope_parameter *param,
                                        option_fields_t *options,
                                        struct queue *a, struct queue *b);
static void *send_worker(void *data);
static int send_buffer(int sock_fd, option_fields_t *options, const char *buf,
                       size_t len);

/******************************************************************************
 * static variables
 ******************************************************************************/
static volatile int interrupted = 0;
static int sa_set = 0;
static struct sigaction oldsa;
static struct sigaction sa = {
	.sa_handler = int_handler,
};

static struct queue queue_a = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.started = 0,
	.write_pos = 0,
	.read_pos = 0,
	.buf = NULL,
	.sock_fd = -1,
	.file_fd = NULL,
};
static struct queue queue_b = {
	.mutex = PTHREAD_MUTEX_INITIALIZER,
	.started = 0,
	.write_pos = 0,
	.read_pos = 0,
	.buf = NULL,
	.sock_fd = -1,
	.file_fd = NULL,
};

static struct sockaddr_in srv_addr, srv_addr2, cli_addr;

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
void signal_init(void) {
	if (!sigaction(SIGINT, &sa, &oldsa))
		sa_set = 1;
	else
		fprintf(stderr, "configuring signals failed (non-fatal), %s\n", strerror(errno));
}

void signal_exit(void) {
	if (sa_set) {
		sigaction(SIGINT, &oldsa, NULL);
		sa_set = 0;
	}
}

/*
 * initializes socket according to options
 * returns 0 on success, <0 on error
 */
int connection_init(option_fields_t *options, struct handles *handles)
{
	handles->sock = socket(PF_INET, options->tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
	if (handles->sock < 0) {
		fprintf(stderr, "create socket failed, %s\n", strerror(errno));
		goto error_close;
	}
	if (options->scope_chn == 2) {
		handles->sock2 = socket(PF_INET, options->tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
		if (handles->sock2 < 0) {
			fprintf(stderr, "create socket failed, %s\n", strerror(errno));
			goto error_close;
		}
	}

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(options->address);
	srv_addr.sin_port = htons(options->port);
	memset(&srv_addr2, 0, sizeof(srv_addr2));
	srv_addr2.sin_family = AF_INET;
	srv_addr2.sin_addr.s_addr = srv_addr.sin_addr.s_addr;
	srv_addr2.sin_port = htons(options->port2);

	if (options->mode == server) {
		int optval = 1;

		srv_addr.sin_addr.s_addr = INADDR_ANY;

		if (setsockopt(handles->sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
			fprintf(stderr, "setsockopt failed, %s\n", strerror(errno));
		}

		if (bind(handles->sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr))) {
			fprintf(stderr, "bind failed, %s\n", strerror(errno));
			goto error_close;
		}
		if (listen(handles->sock, 5)) {
			fprintf(stderr, "listen failed, %s\n", strerror(errno));
			goto error_close;
		}

		handles->server_sock = handles->sock;
		handles->sock = -1;
	}

	return 0;

error_close:
	connection_stop(handles);
	return -1;
}

int connection_start(option_fields_t *options, struct handles *handles)
{
	if (options->mode == client) {
		int do_sleep = 0;
		if (handles->sock < 0) {
			handles->sock = socket(PF_INET, options->tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
			if (handles->sock < 0) {
				fprintf(stderr, "create socket failed, %s\n", strerror(errno));
				goto error_close;
			}
			do_sleep = 1;
		}
		if (options->scope_chn == 2 && handles->sock2 < 0) {
			handles->sock2 = socket(PF_INET, options->tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
			if (handles->sock2 < 0) {
				fprintf(stderr, "create socket failed, %s\n", strerror(errno));
				goto error_close;
			}
			do_sleep = 1;
		}
		if (do_sleep)
			usleep(100000); /* sleep 0.1s before reconnecting */

		if (connect(handles->sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
			fprintf(stderr, "connect failed, %s\n", strerror(errno));
			goto error_close;
		}
		if (options->scope_chn == 2 &&
		    connect(handles->sock2, (struct sockaddr *)&srv_addr2, sizeof(srv_addr2)) < 0) {
			fprintf(stderr, "connect failed, %s\n", strerror(errno));
			goto error_close;
		}
	} else {
		socklen_t cli_len = sizeof(cli_addr);

		handles->sock = accept(handles->server_sock, (struct sockaddr *)&cli_addr, &cli_len);
		if (handles->sock < 0) {
			fprintf(stderr, "accept failed, %s\n", strerror(errno));
			goto error;
		}
	}

	return 0;

error_close:
	connection_stop(handles);
error:
	return -1;
}

void connection_stop(struct handles *handles)
{
	if (handles->sock >= 0) {
		close(handles->sock);
		handles->sock = -1;
	}
	if (handles->sock2 >= 0) {
		close(handles->sock2);
		handles->sock2 = -1;
	}
}

void connection_cleanup(struct handles *handles)
{
	connection_stop(handles);

	if (handles->server_sock >= 0) {
		close(handles->server_sock);
		handles->server_sock = -1;
	}
}

/*
 * opens files according to options
 * returns 0 on success, <0 on error
 */
int file_open(option_fields_t *options, struct handles *handles)
{
	if (!(handles->file = fopen(options->fname, "wb"))) {
		fprintf(stderr, "file open failed, %s\n", strerror(errno));
		goto error_close;
	}
	if (options->scope_chn == 2) {
		if (!(handles->file2 = fopen(options->fname2, "wb"))) {
			fprintf(stderr, "file open failed, %s\n", strerror(errno));
			goto error_close;
		}
	}

	return 0;

error_close:
	file_close(handles);
	return -1;
}

void file_close(struct handles *handles)
{
	if (handles->file) {
		fclose(handles->file);
		handles->file = NULL;
	}
	if (handles->file2) {
		fclose(handles->file2);
		handles->file2 = NULL;
	}
}

int transfer_data(struct scope_parameter *param, option_fields_t *options, struct handles *handles)
{
	unsigned long duration = 0UL;
	u_int64_t transferred = 0ULL;
	struct timeval start_time, end_time;
	int report_rate = options->report_rate;

	if (options->scope_chn == 2 && setup_threads(handles) < 0) {
		teardown_threads();
		return -1;
	}

	if (report_rate && gettimeofday(&start_time, NULL))
		report_rate = 0;

	if (0) /* TODO depending on mmap success */
		transferred = transfer_readwrite(param, options, handles);
	else if (options->scope_chn == 2)
		transferred = transfer_buf_mmap_dual(param, options, &queue_a, &queue_b);
	else
		transferred = transfer_buf_mmap(param, options, handles);

	if (report_rate && !gettimeofday(&end_time, NULL)) {
		duration = 1000UL * (end_time.tv_sec - start_time.tv_sec)
		         + (unsigned long)end_time.tv_usec   / 1000UL
		         - (unsigned long)start_time.tv_usec / 1000UL;
		printf("transferred %lluB in %lums (rate %.2fMB/s)\n",
		       transferred, duration,
		       (double)(1000ULL * transferred) / (1024ULL * 1024ULL * duration));
	}

	if (options->scope_chn == 2)
		teardown_threads();

	return 0;
}

int transfer_interrupted(void)
{
	return interrupted;
}

/******************************************************************************
 * static function definitions
 ******************************************************************************/
static void int_handler(int sig)
{
	interrupted = 1;
}

static int setup_threads(struct handles *handles)
{
	int rc;

	/* allocate cacheable buffers */
	queue_a.buf = malloc(BUFFER_SIZE);
	queue_b.buf = malloc(BUFFER_SIZE);
	if (queue_a.buf == NULL || queue_b.buf == NULL) {
		fprintf(stderr, "malloc failed, %s - buf a %p buf b %p\n",
		        strerror(errno), queue_a.buf, queue_b.buf);
		return -1;
	}

	queue_a.sock_fd = handles->sock;
	queue_b.sock_fd = handles->sock2;
	queue_a.file_fd = handles->file;
	queue_b.file_fd = handles->file2;

	/* start socket senders */
	rc = pthread_create(&queue_a.sender, NULL, send_worker, &queue_a);
	if (rc != 0) {
		fprintf(stderr, "start sender A failed, %s\n", strerror(rc));
		return -1;
	}
	queue_a.started = 1;

	rc = pthread_create(&queue_b.sender, NULL, send_worker, &queue_b);
	if (rc != 0) {
		fprintf(stderr, "start sender B failed, %s\n", strerror(rc));
		return -1;
	}
	queue_b.started = 1;

	return 0;
}

static void teardown_threads()
{
	/* cleanup */
	if (queue_a.started) {
		pthread_cancel(queue_a.sender);
		pthread_join(queue_a.sender, NULL);
	}
	if (queue_b.started) {
		pthread_cancel(queue_b.sender);
		pthread_join(queue_b.sender, NULL);
	}

	if (queue_a.buf)
		free(queue_a.buf);
	if (queue_b.buf)
		free(queue_b.buf);
}

/*
 * transfers samples to socket via read() call on rpad_scope
 */
static u_int64_t transfer_readwrite(struct scope_parameter *param,
                                    option_fields_t *options,
                                    struct handles *handles)
{
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	int block_length;
	char buffer[16384];

	while (!size || transferred < size) {
		if (!size)
			block_length = sizeof(buffer);
		else
			block_length = min(sizeof(buffer), size - transferred);

		block_length = read(param->scope_fd, buffer, block_length);
		if (block_length < 0 || interrupted) {
			if (!interrupted)
				fprintf(stderr, "rpad read failed, %s\n", strerror(errno));
			break;
		}

		if (send_buffer(handles->sock, options, buffer, block_length) < 0) {
			if (!interrupted)
				fprintf(stderr, "socket write failed, %s\n", strerror(errno));
			break;
		}

		transferred += block_length;
	}

	return transferred;
}

/*
 * transfers samples to socket or file from non-cacheable mmap'ed scope buffer via a
 * cacheable intermediate buffer
 */
static u_int64_t transfer_buf_mmap(struct scope_parameter *param,
                                   option_fields_t *options,
                                   struct handles *handles)
{
	const int CHUNK = 8 * 4096;
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	unsigned long pos;
	size_t len;
	unsigned long curr;
	unsigned long *curr_addr;
	unsigned long base;
	void *mapped_base;
	size_t buf_size;
	void *buf;

	if (!(buf = malloc(CHUNK))) {
		fprintf(stderr, "no memory for temp buffer\n");
		return 0ULL;
	}

	curr_addr = param->mapped_io + (options->scope_chn ? 0x118 : 0x114);
	base = *(unsigned long *)(param->mapped_io +
	                          (options->scope_chn ? 0x10c : 0x104));
	mapped_base = options->scope_chn ? param->mapped_buf_b
	                                 : param->mapped_buf_a;
	buf_size = options->scope_chn ? param->buf_b_size : param->buf_a_size;

	pos = 0;
	while (!size || transferred < size) {
		if (pos == buf_size)
			pos = 0;

		curr = *curr_addr - base;

		if (pos + CHUNK <= curr) {
			len = CHUNK;
		} else if (pos > curr) {
			if (pos + CHUNK <= buf_size) {
				len = CHUNK;
			} else {
				len = buf_size - pos;
			}
		} else {
			continue;
		}

		/* copy to cacheable buffer, shortens socket and file overhead by ~75% */
		memcpy(buf, mapped_base + pos, len);

		if (handles->sock >= 0) {
			if (send_buffer(handles->sock, options, buf, len) < 0) {
				if (!interrupted)
					fprintf(stderr, "socket write failed, %s\n", strerror(errno));
				break;
			}
		} else {
			len = fwrite(buf, 1, len, handles->file);
			if (len <= 0 || interrupted) {
				if (!interrupted)
					fprintf(stderr, "file write failed, %s\n", strerror(errno));
				break;
			}
		}

		pos += len;
		transferred += len;
	}

	free(buf);

	return transferred;
}

/*
 * reads samples from dma ram and puts them on the channel queues if enough free
 * space is available on the queue (measured by the difference between
 * queue->read_pos and queue->write_pos). advances each queue's write_pos for
 * each block that was copied. rinse and repeat. access to write_pos and
 * read_pos is protected by queue->mutex.
 */
static u_int64_t transfer_buf_mmap_dual(struct scope_parameter *param,
                                        option_fields_t *options,
                                        struct queue *a, struct queue *b)
{
	u_int64_t transferred = 0ULL;
	u_int64_t size = 1024ULL * options->kbytes_to_transfer;
	unsigned long pos_a = 0, pos_b = 0;
	unsigned long curr_a, curr_b;
	unsigned long *curr_addr_a, *curr_addr_b;
	unsigned long base_a, base_b;
	void *mapped_base_a, *mapped_base_b;
	size_t buf_size_a, buf_size_b;
	size_t len;
	unsigned int read_pos_a, read_pos_b;
	unsigned int write_pos_a = 0, write_pos_b = 0;
	int did_something;

	curr_addr_a = param->mapped_io + 0x114;
	curr_addr_b = param->mapped_io + 0x118;
	base_a = *(unsigned long *)(param->mapped_io + 0x104);
	base_b = *(unsigned long *)(param->mapped_io + 0x10c);
	mapped_base_a = param->mapped_buf_a;
	mapped_base_b = param->mapped_buf_b;
	buf_size_a = param->buf_a_size;
	buf_size_b = param->buf_b_size;

	while (!interrupted && (!size || transferred < size)) {
		did_something = 0;

		if (pthread_mutex_lock(&a->mutex) != 0)
			break;
		a->write_pos = write_pos_a;
		read_pos_a = a->read_pos;
		if (pthread_mutex_unlock(&a->mutex) != 0)
			break;

		if (CIRCULAR_DIST(read_pos_a, write_pos_a, BUFFER_SIZE) < BUFFER_SIZE - BLOCK_SIZE) {
			curr_a = *curr_addr_a - base_a;

			len = CIRCULAR_DIST(pos_a, curr_a, buf_size_a);

			if (len > 0) {
				if (len > BLOCK_SIZE)
					len = BLOCK_SIZE;
				CIRCULAR_MEMCPY(a->buf, write_pos_a, BUFFER_SIZE, mapped_base_a, pos_a, buf_size_a, len);
				pos_a = CIRCULAR_ADD(pos_a, len, buf_size_a);
				write_pos_a = CIRCULAR_ADD(write_pos_a, len, BUFFER_SIZE);
				transferred += len;
				did_something = 1;
			}
		}

		if (pthread_mutex_lock(&b->mutex) != 0)
			break;
		b->write_pos = write_pos_b;
		read_pos_b = b->read_pos;
		if (pthread_mutex_unlock(&b->mutex) != 0)
			break;

		if (CIRCULAR_DIST(read_pos_b, write_pos_b, BUFFER_SIZE) < BUFFER_SIZE - BLOCK_SIZE) {
			curr_b = *curr_addr_b - base_b;

			len = CIRCULAR_DIST(pos_b, curr_b, buf_size_b);

			if (len > 0) {
				if (len > BLOCK_SIZE)
					len = BLOCK_SIZE;
				CIRCULAR_MEMCPY(b->buf, write_pos_b, BUFFER_SIZE, mapped_base_b, pos_b, buf_size_b, len);
				pos_b = CIRCULAR_ADD(pos_b, len, buf_size_b);
				write_pos_b = CIRCULAR_ADD(write_pos_b, len, BUFFER_SIZE);
				transferred += len;
				did_something = 1;
			}
		}

		if (!did_something)
			usleep(5);
	}

	return transferred;
}

/*
 * sends samples from a struct queue. synchronisation with the queue is done via
 * queue->write_pos and queue->read_pos. send_worker will send a block of
 * BLOCK_SIZE bytes whenever the queue holds enough data (measured by the
 * difference between write_pos and read_pos) and advance read_pos accordingly.
 * access to write_pos and read_pos is protected by queue->mutex.
 */
static void *send_worker(void *data)
{
	struct queue *q = (struct queue *)data;
	unsigned int send_pos = 0;
	unsigned int write_pos;
	ssize_t sent;
	size_t length;

	do {
		if (pthread_mutex_lock(&q->mutex) != 0)
			goto send_worker_exit;
		write_pos = q->write_pos;
		q->read_pos = send_pos;
		if (pthread_mutex_unlock(&q->mutex) != 0)
			goto send_worker_exit;

		if (CIRCULAR_DIST(send_pos, write_pos, BUFFER_SIZE) >= BLOCK_SIZE) {
			length = BLOCK_SIZE;
			do {
				if (q->sock_fd >= 0)
					sent = send(q->sock_fd, q->buf + send_pos, length, MSG_NOSIGNAL);
				else
					sent = fwrite(q->buf + send_pos, 1, length, q->file_fd);
				if (sent > 0) {
					send_pos += sent;
					length -= sent;
				}
			} while (sent >= 0 && length > 0);

			if (sent < 0)
				goto send_worker_exit;

			send_pos = CIRCULAR(send_pos, BUFFER_SIZE);
		} else {
			usleep(5);
		}
	} while (1);

send_worker_exit:
	interrupted = 1;
	return NULL;
}

static int send_buffer(int sock_fd, option_fields_t *options, const char *buf, size_t len)
{
	int retval = 0;
	int sent;
	unsigned int pos;

	for (pos = 0; pos < len; pos += sent) {
		sent = send(sock_fd, buf + pos, len - pos, MSG_NOSIGNAL);
		if (sent < 0 || interrupted) {
			retval = -1;
			break;
		}
	}

	return retval;
}
