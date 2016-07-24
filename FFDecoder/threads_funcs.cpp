/*****************************************************************************
 * thread_funcs.cpp: thread/mutex/cond/time source file for LuberVideoChat
 *****************************************************************************
 *
 * Copyright (C) 2011-2012 Luber Team
 * Authors : Luber Zheng
 * 
 *****************************************************************************/

#include "stdafx.h"

#include <process.h>
#include "threads_funcs.h"

int mutex_init(mutex_t *p_mutex)
{
    if(!p_mutex)
        return 0;
    p_mutex->mutex = CreateMutex( 0, FALSE, 0 );
    return ( p_mutex->mutex != NULL ? 0 : 1 );
}

mutex_t * mutex_init()
{
    mutex_t *p_mutex = new mutex_t;
    if (p_mutex)
        p_mutex->mutex = CreateMutex( 0, FALSE, 0 );
    else
        return NULL;
    return p_mutex;
}

int mutex_destroy(mutex_t *p_mutex)
{
    if( p_mutex && p_mutex->mutex )
    {
        CloseHandle( p_mutex->mutex );
    }
    return 0;
}

int mutex_lock(mutex_t *p_mutex)
{
    if( p_mutex && p_mutex->mutex )
    {
        WaitForSingleObject( p_mutex->mutex, INFINITE );
    }
    return 0;
}

int mutex_unlock(mutex_t *p_mutex)
{
    if( p_mutex && p_mutex->mutex )
    {
        ReleaseMutex( p_mutex->mutex );
    }
    return 0;
}

int cond_init(cond_t *p_condvar)
{
    if(!p_condvar) return 0;
    p_condvar->i_waiting_threads = 0;

    /* Create an auto-reset event. */
    p_condvar->event = CreateEvent( NULL,   /* no security */
                                    FALSE,  /* auto-reset event */
                                    FALSE,  /* start non-signaled */
                                    NULL ); /* unnamed */

    p_condvar->semaphore = NULL;

    HINSTANCE hInstLib;
    if( GetVersion() < 0x80000000 )
    {
        /* We are running on NT/2K/XP, we can use SignalObjectAndWait */
        hInstLib = LoadLibrary( _T("kernel32") );
        if( hInstLib )
        {
            p_condvar->SignalObjectAndWait =
                (SIGNALOBJECTANDWAIT)GetProcAddress( hInstLib,
                                                 "SignalObjectAndWait" );
            FreeLibrary(hInstLib);
        }
    }
    else
        p_condvar->SignalObjectAndWait = NULL;

    return !p_condvar->event;
}

cond_t* cond_init()
{
    cond_t *p_condvar;
    p_condvar = new cond_t;
    if (!p_condvar) 
        return NULL;

    p_condvar->i_waiting_threads = 0;

    /* Create an auto-reset event. */
    p_condvar->event = CreateEvent( NULL,   /* no security */
                                    FALSE,  /* auto-reset event */
                                    FALSE,  /* start non-signaled */
                                    NULL ); /* unnamed */

    p_condvar->semaphore = NULL;

    HINSTANCE hInstLib;
    if( GetVersion() < 0x80000000 )
    {
        /* We are running on NT/2K/XP, we can use SignalObjectAndWait */
        hInstLib = LoadLibrary( _T("kernel32") );
        if( hInstLib )
        {
            p_condvar->SignalObjectAndWait =
                (SIGNALOBJECTANDWAIT)GetProcAddress( hInstLib,
                                                 "SignalObjectAndWait" );
            FreeLibrary(hInstLib);
        }
    }
    else
        p_condvar->SignalObjectAndWait = NULL;

    return p_condvar;
}

int cond_destroy(cond_t *p_condvar)
{
    int i_result;
    if(!p_condvar) return 0;
    i_result = !CloseHandle( p_condvar->event );
    return i_result;
}

int cond_signal(cond_t *p_condvar)
{
    /* In case of error : */
    int i_thread = -1;
    const char * psz_error = "";
    if(!p_condvar) return 0;
    PulseEvent( p_condvar->event );
    return 0;
}

int cond_broadcast(cond_t *p_condvar)
{
    /* In case of error : */
    int i_thread = -1;
    const char * psz_error = "";
    int i;
    if(!p_condvar) return 0;
    for( i = p_condvar->i_waiting_threads; i > 0; i-- )
    {
        PulseEvent( p_condvar->event );
    }
    if( p_condvar->i_waiting_threads )
    {
        ReleaseSemaphore( p_condvar->semaphore,
                          p_condvar->i_waiting_threads, 0 );

        /* Wait for the last thread to be awakened */
        WaitForSingleObject( p_condvar->event, INFINITE );
    }

    return 0;
}

int cond_wait(cond_t *p_condvar, mutex_t *p_mutex)
{
    /* In case of error : */
    int i_thread = -1;
    const char * psz_error = "";
    if(!p_condvar || !p_mutex) return 0;
    /* Increase our wait count */
    p_condvar->i_waiting_threads++;
    if( p_mutex->mutex )
    {
        p_condvar->SignalObjectAndWait( p_mutex->mutex,
                                        p_condvar->event,
                                        INFINITE, FALSE );
    }
    p_condvar->i_waiting_threads--;
    mutex_lock( p_mutex );

    return 0;
}


CRITICAL_SECTION    date_lock;
mtime_t             i_previous_time = I64C(-1);
int                 i_wrap_counts = -1;
mtime_t             i64_base_clock = 0;

////////////////////////////////////////////////
mtime_t mdate( void )// micro seconde  1000 000 = 1.0 S
{
    static mtime_t freq = I64C(-1);
    
    mtime_t    usec_time;
    mtime_t    second;
    mtime_t    microsecond;

    if( freq == I64C(-1) )
    {
#if 0
        freq = ( QueryPerformanceFrequency( (LARGE_INTEGER *)&freq ) &&
                 (freq == I64C(1193182) || freq == I64C(3579545) ) )
               ? freq : 0;
#else
        BOOL        b_use;
        b_use =  QueryPerformanceFrequency( (LARGE_INTEGER *)&freq );
        if(b_use == FALSE)
        {
            freq = 0;
        }
#endif
    }
    freq = 0;//JCS

    if( freq != 0 )
    {
        if( i_wrap_counts == -1 )
        {
            InitializeCriticalSection( &date_lock );
            i_wrap_counts = 0;
        }

        /* Microsecond resolution */
        EnterCriticalSection( &date_lock );
        
        QueryPerformanceCounter( (LARGE_INTEGER *)&usec_time );
        if(i64_base_clock==0)
        {
            i64_base_clock = usec_time;
        }else
        if(i64_base_clock > usec_time)
        {
            i64_base_clock = usec_time;
        }
        
        usec_time -= i64_base_clock;
        second = usec_time / freq;
        microsecond = usec_time - (second * freq);
        usec_time = second * 1000000 + microsecond * 1000000/freq + 1;
        LeaveCriticalSection( &date_lock );
        return usec_time;
    }
    else
    {
        /* Fallback on GetTickCount() which has a milisecond resolution
         * (actually, best case is about 10 ms resolution)
         * GetTickCount() only returns a DWORD thus will wrap after
         * about 49.7 days so we try to detect the wrapping. */


        if( i_wrap_counts == -1 )
        {
            /* Initialization */
            i_previous_time = I64C(1000) * GetTickCount();
            InitializeCriticalSection( &date_lock );
            i_wrap_counts = 0;
        }

        EnterCriticalSection( &date_lock );
        usec_time = I64C(1000) *
            (i_wrap_counts * I64C(0x100000000) + GetTickCount());
        if( i_previous_time > usec_time )
        {
            /* Counter wrapped */
            i_wrap_counts++;
            usec_time += I64C(0x100000000000);
        }
        i_previous_time = usec_time;
        LeaveCriticalSection( &date_lock );

        return usec_time;
    }
}

void mwait( mtime_t date )
{
    mtime_t    usec_time, delay;

    usec_time = mdate();
    delay = date - usec_time;
    if( delay <= 0 )
    {
        return;
    }
    msleep( delay );
}

void msleep( mtime_t delay)
{
    if( delay > 1000000 )
    {
        delay = 1000000 ;
    }else
    if( delay < 0)
        delay = 0;
    Sleep( (int) (delay / 1000) );
}

void JoinThread(HANDLE& thread_id, int delay)
{
    WaitForSingleObject(thread_id, INFINITE);
    CloseHandle(thread_id);
}

void * fast_memcpy( void * _to, const void * _from, size_t len )
{
    if(_to == NULL || _from == NULL || len == 0)
        return _to;
    memcpy(_to,_from,len);
    return _to;
}
