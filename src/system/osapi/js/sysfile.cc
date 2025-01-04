#include "system/file.h"

int sys_canonicalize(char *result, const char *filename) {
    // Stub implementation
    return 0;
}

int sys_file_mode(int mode) {
    // Stub implementation
    return 0;
}

int sys_findclose(pfind_t &pfind) {
    // Stub implementation
    return 0;
}

int sys_findfirst(pfind_t &pfind, const char *dirname) {
    // Stub implementation
    return 0;
}

int sys_findnext(pfind_t &pfind) {
    // Stub implementation
    return 0;
}

int sys_pstat(pstat_t &s, const char *filename) {
    // Stub implementation
    return 0;
}

int sys_pstat_fd(pstat_t &s, int fd) {
    // Stub implementation
    return 0;
}

int sys_truncate(const char *filename, FileOfs ofs) {
    // Stub implementation
    return 0;
}

int sys_truncate_fd(int fd, FileOfs ofs) {
    // Stub implementation
    return 0;
}

int sys_deletefile(const char *filename) {
    // Stub implementation
    return 0;
}

bool sys_is_path_delim(char c) {
    // Stub implementation
    return false;
}

int sys_filename_cmp(const char *a, const char *b) {
    // Stub implementation
    return 0;
}

bool sys_filename_is_absolute(const char *filename) {
    // Stub implementation
    return false;
}

SYS_FILE *sys_fopen(const char *filename, int openmode) {
    // Stub implementation
    return nullptr;
}

void sys_fclose(SYS_FILE *file) {
    // Stub implementation
}

int sys_fread(SYS_FILE *file, byte *buf, int size) {
    // Stub implementation
    return 0;
}

int sys_fwrite(SYS_FILE *file, byte *buf, int size) {
    // Stub implementation
    return 0;
}

int sys_fseek(SYS_FILE *file, FileOfs newofs, int seekmode) {
    // Stub implementation
    return 0;
}

FileOfs sys_ftell(SYS_FILE *file) {
    // Stub implementation
    return 0;
}

void sys_flush(SYS_FILE *file) {
    // Stub implementation
}

void sys_suspend()
{
    // TODO: idlewait and/or sleep
}
