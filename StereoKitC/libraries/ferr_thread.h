/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2023 Nick Klingensmith
 * Copyright (c) 2023 Qualcomm Technologies, Inc.
 */

#pragma once

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)

	#define FT_WIN

#else

	#define FT_POSIX
	#include <pthread.h>

#endif

#include <stdint.h>
#include <stdbool.h>

///////////////////////////////////////////

struct  _ft_mutex_t;
typedef _ft_mutex_t* ft_mutex_t;

struct  _ft_condition_t;
typedef _ft_condition_t* ft_condition_t;

#if defined(FT_WIN)
typedef void*     ft_thread_t;
#else
typedef pthread_t ft_thread_t;
#endif

#if defined(FT_WIN)
typedef uint32_t  ft_id_t;
#else
typedef pthread_t ft_id_t;
#endif

///////////////////////////////////////////

ft_mutex_t     ft_mutex_create       (void);
void           ft_mutex_destroy      (ft_mutex_t *mutex);
void           ft_mutex_lock         (ft_mutex_t  mutex);
void           ft_mutex_unlock       (ft_mutex_t  mutex);

ft_condition_t ft_condition_create   (void);
void           ft_condition_destroy  (ft_condition_t *condition);
void           ft_condition_wait     (ft_condition_t  condition, ft_mutex_t wait_mutex);
void           ft_condition_broadcast(ft_condition_t  condition);
void           ft_condition_signal   (ft_condition_t  condition);

bool           ft_id_matches         (ft_id_t thread_id);
bool           ft_id_equal           (ft_id_t a, ft_id_t b);
ft_id_t        ft_id_current         (void);

ft_thread_t    ft_thread_create      (int32_t (*thread_func)(void *args), void *args);
ft_thread_t    ft_thread_current     (void);
void           ft_thread_name        (ft_thread_t thread, const char* name);

void           ft_yield              (void);

///////////////////////////////////////////

#ifdef FERR_THREAD_IMPL

#if defined(FT_WIN)

	#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>

	struct _ft_mutex_t {
		CRITICAL_SECTION section;
	};
	struct _ft_condition_t {
		CONDITION_VARIABLE condition;
	};

#else

	#include <pthread.h>
	struct _ft_mutex_t {
		pthread_mutex_t mutex;
	};
	struct _ft_condition_t {
		pthread_cond_t condition;
	};

#endif

#if __EMSCRIPTEN__

	#include <emscripten/threading.h>

#endif

#include <stdlib.h>

///////////////////////////////////////////
// ft_mutex_t                            //
///////////////////////////////////////////

ft_mutex_t ft_mutex_create() {
	_ft_mutex_t *result = (_ft_mutex_t*)malloc(sizeof(_ft_mutex_t));
	if (result == nullptr) return nullptr;
	*result = {};

#if defined(FT_WIN)
	InitializeCriticalSection(&result->section);

#else
	if (pthread_mutex_init(&result->mutex, nullptr) != 0) {
		free(result);
		return nullptr;
	}

#endif

	return result;
}

///////////////////////////////////////////

void ft_mutex_destroy(ft_mutex_t *mutex) {
	if (mutex == nullptr || *mutex == nullptr) return;

#if defined(FT_WIN)
	DeleteCriticalSection(&(*mutex)->section);
#else
	pthread_mutex_destroy(&(*mutex)->mutex);
#endif

	free(*mutex);
	*mutex = {};
}

///////////////////////////////////////////

void ft_mutex_lock(ft_mutex_t mutex) {
#if defined(FT_WIN)
	EnterCriticalSection(&mutex->section);
#else
	pthread_mutex_lock(&mutex->mutex);
#endif
}

///////////////////////////////////////////

void ft_mutex_unlock(ft_mutex_t mutex) {
#if defined(FT_WIN)
	LeaveCriticalSection(&mutex->section);
#else
	pthread_mutex_unlock(&mutex->mutex);
#endif
}

///////////////////////////////////////////
// ft_condition_t                        //
///////////////////////////////////////////

ft_condition_t ft_condition_create() {
	_ft_condition_t* result = (_ft_condition_t*)malloc(sizeof(_ft_condition_t));
	if (result == nullptr) return nullptr;
	*result = {};

#if defined(FT_WIN)
	InitializeConditionVariable(&result->condition);
#else
	if (pthread_cond_init(&result->condition, nullptr) != 0) {
		free(result);
		return nullptr;
	}
#endif

	return result;
}

///////////////////////////////////////////

void ft_condition_destroy(ft_condition_t* condition) {
	if (condition == nullptr || *condition == nullptr) return;

#if defined(FT_WIN)
#else
	pthread_cond_destroy(&(*condition)->condition);
#endif
	free(*condition);
	*condition = {};
}

///////////////////////////////////////////

void ft_condition_wait(ft_condition_t condition, ft_mutex_t wait_mutex) {
#if defined(FT_WIN)
	SleepConditionVariableCS(&condition->condition, &wait_mutex->section, INFINITE);
#else
	pthread_cond_wait(&condition->condition, &wait_mutex->mutex);
#endif
}

///////////////////////////////////////////

void ft_condition_broadcast(ft_condition_t condition) {
#if defined(FT_WIN)
	WakeAllConditionVariable(&condition->condition);
#else
	pthread_cond_broadcast(&condition->condition);
#endif
}

///////////////////////////////////////////

void ft_condition_signal(ft_condition_t condition) {
#if defined(FT_WIN)
	WakeConditionVariable(&condition->condition);
#else
	pthread_cond_signal(&condition->condition);
#endif
}

///////////////////////////////////////////
// ft_id_t                               //
///////////////////////////////////////////

bool ft_id_matches(ft_id_t thread_id) {
#if defined(FT_WIN)
	return thread_id == GetCurrentThreadId();
#else
	return pthread_equal(thread_id, pthread_self());
#endif
}

///////////////////////////////////////////

ft_id_t ft_id_current() {
#if defined(FT_WIN)
	return GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

///////////////////////////////////////////

bool ft_id_equal(ft_id_t a, ft_id_t b) {
#if defined(FT_WIN)
	return a == b;
#else
	return pthread_equal(a, b);
#endif
}

///////////////////////////////////////////
// ft_thread_t                           //
///////////////////////////////////////////

struct _ft_thread_info_t {
	int32_t (*func)(void* args);
	void* args;
};

#if defined(FT_WIN)
DWORD WINAPI _ft_thread_wrapper(LPVOID args)
#else
void* _ft_thread_wrapper(void* args)
#endif
{
	_ft_thread_info_t* info   = (_ft_thread_info_t*)args;
	int32_t            result = info->func(info->args);
	free(args);

#if defined(FT_WIN)
	return result;
#else
	return (void*)(int64_t)result;
#endif 
}

///////////////////////////////////////////

ft_thread_t ft_thread_create(int32_t (*thread_func)(void* args), void* args) {
	_ft_thread_info_t* thread_info = (_ft_thread_info_t*)malloc(sizeof(_ft_thread_info_t));
	thread_info->func = thread_func;
	thread_info->args = args;

#if defined(FT_WIN)
	return (ft_thread_t)CreateThread(nullptr, 0, _ft_thread_wrapper, thread_info, 0, nullptr);
#else
	pthread_t result = {};
	pthread_create(&result, nullptr, _ft_thread_wrapper, thread_info);
	return result;
#endif
}

///////////////////////////////////////////

ft_thread_t ft_thread_current() {
#if defined(FT_WIN)
	return GetCurrentThread();
#else
	return pthread_self();
#endif
}

///////////////////////////////////////////

void ft_thread_name(ft_thread_t thread, const char* name) {
#if defined(FT_WIN)
	// convert to wchar_t
	size_t   name_len  = strlen(name);
	wchar_t* name_wide = (wchar_t*)malloc((name_len + 1) * sizeof(wchar_t));
	mbstowcs(name_wide, name, name_len);
	name_wide[name_len] = 0;

	SetThreadDescription(thread, name_wide);
	free(name_wide);
#elif defined(__EMSCRIPTEN__)
	emscripten_set_thread_name(thread, name);
#elif defined(__APPLE__)
	// macOS pthread_setname_np only works on the current thread
	if (pthread_equal(thread, pthread_self())) {
		pthread_setname_np(name);
	}
#else
	pthread_setname_np(thread, name);
#endif
}

///////////////////////////////////////////

void ft_yield(void) {
#if defined(FT_WIN)
	Sleep(0);
#else
	sched_yield();
#endif
}

#endif // FERR_THREAD_IMPL