/*
 * Copyright 2009 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef HAVE_CONFIG_H
#include "event-config.h"
#endif

#include <pthread.h>
struct event_base;
#include <event2/thread.h>

#include "mm-internal.h"

static void *
evthread_posix_lock_create(void)
{
	pthread_mutex_t *lock = mm_malloc(sizeof(pthread_mutex_t));
	if (!lock)
		return NULL;
	pthread_mutex_init(lock, NULL);
	return lock;
}

static void
evthread_posix_lock_free(void *_lock)
{
	pthread_mutex_t *lock = _lock;
	pthread_mutex_destroy(lock);
	mm_free(lock);
}

static void
evthread_posix_lock(int mode, void *_lock)
{
	pthread_mutex_t *lock = _lock;
	if (0 != (mode & EVTHREAD_LOCK))
		pthread_mutex_lock(lock);
	else
		pthread_mutex_unlock(lock);
}

static unsigned long
evthread_posix_get_id(void)
{
	union {
		pthread_t thr;
		unsigned long id;
	} r;
	r.thr = pthread_self();
	return r.id;
}

int
evthread_use_pthreads(struct event_base *base)
{
	evthread_set_lock_create_callbacks(base,
									   evthread_posix_lock_create,
									   evthread_posix_lock_free);
	evthread_set_locking_callback(base, evthread_posix_lock);
	evthread_set_id_callback(base, evthread_posix_get_id);
	return -1;
}