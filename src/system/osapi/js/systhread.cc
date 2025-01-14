#include <cstdio>

#include "system/systhread.h"

//#define SYS_THREAD_TRACE(msg...) ht_printf("[sys/thread] " msg)
#define SYS_THREAD_TRACE(msg...) 

int sys_create_mutex(sys_mutex *m) {
    SYS_THREAD_TRACE("sys_create_mutex()\n");
    return 0;
}

int sys_create_semaphore(sys_semaphore *s) {
    SYS_THREAD_TRACE("sys_create_semaphore()\n");
    return 0;
}

int sys_create_thread(sys_thread *t, int flags, sys_thread_function start_routine, void *arg) {
    SYS_THREAD_TRACE("sys_create_thread()\n");
    return 0;
}

void sys_destroy_mutex(sys_mutex m) {
    SYS_THREAD_TRACE("sys_destroy_mutex()\n");
}

void sys_destroy_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_destroy_semaphore()\n");
}

void sys_destroy_thread(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_destroy_thread()\n");
}

int sys_lock_mutex(sys_mutex m) {
    return 0;
}

int sys_trylock_mutex(sys_mutex m) {
    return 0;
}

void sys_unlock_mutex(sys_mutex m) {
}

void sys_signal_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_signal_semaphore()\n");
}

void sys_signal_all_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_signal_all_semaphore()\n");
}

void sys_wait_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_wait_semaphore()\n");
}

void sys_wait_semaphore_bounded(sys_semaphore s, int ms) {
    SYS_THREAD_TRACE("sys_wait_semaphore_bounded()\n");
}

void sys_lock_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_lock_semaphore()\n");
}

void sys_unlock_semaphore(sys_semaphore s) {
    SYS_THREAD_TRACE("sys_unlock_semaphore()\n");
}

void sys_exit_thread(void *ret) {
    SYS_THREAD_TRACE("sys_exit_thread()\n");
}

void *sys_join_thread(sys_thread t) {
    SYS_THREAD_TRACE("sys_join_thread()\n");
    return nullptr;
}
