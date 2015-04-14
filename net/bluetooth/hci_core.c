/* hci_core.c - HCI core Bluetooth handling */

/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3) Neither the name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <nanokernel.h>
#include <toolchain.h>
#include <string.h>
#include <errno.h>
#include <misc/byteorder.h>

#include <bluetooth/bluetooth.h>

/* Available (free) buffers queue */
#define NUM_BUFS		5
static struct bt_buf		buffers[NUM_BUFS];
static struct nano_fifo		free_bufs;

struct bt_buf *bt_buf_get_reserve(size_t reserve_head)
{
	struct bt_buf *buf;

	buf = nano_fifo_get(&free_bufs);
	if (!buf) {
		BT_ERR("Failed to get free buffer\n");
		return NULL;
	}

	buf->data = buf->buf + reserve_head;
	buf->len = 0;
	buf->sync = NULL;

	BT_DBG("buf %p reserve %u\n", buf, reserve_head);

	return buf;
}

struct bt_buf *bt_buf_get(void)
{
	return bt_buf_get_reserve(0);
}

void bt_buf_put(struct bt_buf *buf)
{
	BT_DBG("buf %p\n", buf);

	nano_fifo_put(&free_bufs, buf);
}

uint8_t *bt_buf_add(struct bt_buf *buf, size_t len)
{
	uint8_t *tail = buf->data + buf->len;
	buf->len += len;
	return tail;
}

uint8_t *bt_buf_push(struct bt_buf *buf, size_t len)
{
	buf->data -= len;
	buf->len += len;
	return buf->data;
}

uint8_t *bt_buf_pull(struct bt_buf *buf, size_t len)
{
	buf->len -= len;
	return buf->data += len;
}

size_t bt_buf_headroom(struct bt_buf *buf)
{
	return buf->data - buf->buf;
}

size_t bt_buf_tailroom(struct bt_buf *buf)
{
	return BT_BUF_MAX_DATA - bt_buf_headroom(buf) - buf->len;
}

static void init_free_queue(void)
{
	nano_fifo_init(&free_bufs);

	for (int i = 0; i < NUM_BUFS; i++)
		nano_fifo_put(&free_bufs, &buffers[i]);
}

int bt_init(void)
{
	init_free_queue();

	return 0;
}
