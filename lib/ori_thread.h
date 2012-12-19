#ifndef _ORI_THREAD_H
#define _ORI_THREAD_H

#include "ssh_typedef.h"

void* ori_thread_create(void* (*thread_function)(void* arg), void* arg);
void* ori_thread_mutex_init(void);
void ori_thread_mutex_lock(void* mutex);
void ori_thread_mutex_unlock(void* mutex);
void* ori_thread_codition_init(void);
void ori_thread_condition_wait(void* condition, void* mutex);
void ori_thread_condition_signal(void* condition);
void ori_thread_cancel(void* thread);
Boolean ori_thread_is_cancelled(void* thread);
void ori_thread_join(void* thread);
void ori_thread_free(void* thread);
void ori_thread_mutex_free(void* mutex);
void ori_thread_condition_free(void *mutex);

#endif