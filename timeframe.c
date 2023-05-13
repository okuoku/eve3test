/* 50Hz timeframe */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <windows.h>

/* Exports */
uint64_t timeframe_wait_next(uint64_t prev);
void timeframe_init(void);

/* Globals */
static pthread_cond_t tf_cond;
static pthread_mutex_t tf_mtx;
static uint64_t tf_frame = 0;



uint64_t
timeframe_wait_next(uint64_t prev){
    uint64_t r;
    pthread_mutex_lock(&tf_mtx);
    if(prev == tf_frame){
retry:
        /* Sleep */
        pthread_cond_wait(&tf_cond, &tf_mtx);
        if(prev == tf_frame){
            /* May be interrupted */
            printf("May be interrupted. Try again(%lld).\n", tf_frame);
            goto retry;
        }
    }
    r = tf_frame;
    pthread_mutex_unlock(&tf_mtx);
    return r;
}

static void
timeproc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser,
         DWORD_PTR dw1, DWORD_PTR dw2){
    pthread_mutex_lock(&tf_mtx);
    tf_frame++;
    pthread_cond_signal(&tf_cond);
    pthread_mutex_unlock(&tf_mtx);
}

void
timeframe_init(void){
    MMRESULT r;
    /* Request 1ms timer */
    r = timeBeginPeriod(1);
    if(r != TIMERR_NOERROR){
        printf("Cannot request timer. 0x%x\n",(int)r);
        abort();
    }

    /* Initialize condvar */
    pthread_cond_init(&tf_cond, NULL);
    pthread_mutex_init(&tf_mtx, NULL);

    /* Start timer */
    r = timeSetEvent(20, 1, timeproc, 
                     0, TIME_PERIODIC);
    if(! r){
        printf("Could not request callback.\n");
        abort();
    }
}
