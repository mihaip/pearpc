#include "system/systhread.h"

int sys_create_mutex(sys_mutex *m) {
    return 0;
}

int sys_create_semaphore(sys_semaphore *s) {
    return 0;
}

int sys_create_thread(sys_thread *t, int flags, sys_thread_function start_routine, void *arg) {
    return 0;
}

void sys_destroy_mutex(sys_mutex m) {
    // Stub
}

void sys_destroy_semaphore(sys_semaphore s) {
    // Stub
}

void sys_destroy_thread(sys_semaphore s) {
    // Stub
}

int sys_lock_mutex(sys_mutex m) {
    return 0;
}

int sys_trylock_mutex(sys_mutex m) {
    return 0;
}

void sys_unlock_mutex(sys_mutex m) {
    // Stub
}

void sys_signal_semaphore(sys_semaphore s) {
    // Stub
}

void sys_signal_all_semaphore(sys_semaphore s) {
    // Stub
}

void sys_wait_semaphore(sys_semaphore s) {
    // Stub
}

void sys_wait_semaphore_bounded(sys_semaphore s, int ms) {
    // Stub
}

void sys_lock_semaphore(sys_semaphore s) {
    // Stub
}

void sys_unlock_semaphore(sys_semaphore s) {
    // Stub
}

void sys_exit_thread(void *ret) {
    // Stub
}

void *sys_join_thread(sys_thread t) {
    return nullptr;
}
