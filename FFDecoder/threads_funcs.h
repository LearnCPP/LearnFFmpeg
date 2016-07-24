/*****************************************************************************
 * thread_funcs.h: thread/mutex/cond/time header file for LuberVideoChat
 *****************************************************************************
 *
 * Copyright (C) 2011-2012 Luber Team
 * Authors : Luber Zheng
 * 
 *****************************************************************************/

#ifndef LUBER_THREAD_FUNCS_H
#define LUBER_THREAD_FUNCS_H
#pragma once

#include <windows.h>
#include "inttypes.h"
#include <tchar.h>

//#include "mm_config.h"
typedef BOOL (WINAPI *SIGNALOBJECTANDWAIT) ( HANDLE, HANDLE, DWORD, BOOL );

#define        SDL_mutex mutex_t
#define        SDL_cond cond_t
#define        SDL_CreateMutex mutex_init
#define        SDL_LockMutex mutex_lock
#define        SDL_UnlockMutex mutex_unlock
#define        SDL_DestroyMutex mutex_destroy
#define        SDL_CreateCond cond_init
#define        SDL_DestroyCond cond_destroy
#define        SDL_CondSignal cond_signal
#define        SDL_CondWait cond_wait
#define        SDL_Delay Sleep
#define		   SDL_Overlay void

typedef struct mutex_t
{
    HANDLE              mutex;
    CRITICAL_SECTION    csection;
}mutex_t;

typedef struct cond_t
{
    volatile int        i_waiting_threads;
    HANDLE              event;
    SIGNALOBJECTANDWAIT SignalObjectAndWait;
    HANDLE              semaphore;
    CRITICAL_SECTION    csection;
    int                 i_win9x_cv;
} cond_t;

#define THREAD_PRIORITY_LOW       0
#define THREAD_PRIORITY_INPUT     20        //input demux thread    == 0
#define THREAD_PRIORITY_AUDIO     20        //audio decode thread    == 20
#define THREAD_PRIORITY_VIDEO     15        //video decode thread    == 15
#define THREAD_PRIORITY_VOUT      30        //video out thread          == 30

int    mutex_init(mutex_t     *p_mutex);
//mutex_t * mutex_init();
int    mutex_destroy(mutex_t  *p_mutex);
int    mutex_lock(mutex_t     *p_mutex);
int    mutex_unlock(mutex_t   *p_mutex);

int    cond_init(cond_t       *p_condvar);
//cond_t * cond_init();
int    cond_destroy(cond_t    *p_condvar);
int    cond_signal(cond_t     *p_condvar);
int    cond_broadcast(cond_t  *p_condvar);
int    cond_wait(cond_t       *p_condvar, mutex_t        *p_mutex);
void JoinThread(HANDLE&  thread_id, int delay = 10);

typedef int64_t     mtime_t;
mtime_t             mdate();
void                mwait(mtime_t date);
void                msleep(mtime_t delay);

#endif // LUBER_THREAD_FUNCS_H