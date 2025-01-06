#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <emscripten.h>
#include <sys/stat.h>

#include "system/file.h"

int sys_canonicalize(char *result, const char *filename) {
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	return (realpath(filename, result)==result) ? 0 : ENOENT;
}

int sys_file_mode(int mode) {
    	int m = 0;
	if (S_ISREG(mode)) {
		m |= HT_S_IFREG;
	} else if (S_ISBLK(mode)) {
		m |= HT_S_IFBLK;
	} else if (S_ISCHR(mode)) {
		m |= HT_S_IFCHR;
	} else if (S_ISDIR(mode)) {
		m |= HT_S_IFDIR;
	} else if (S_ISFIFO(mode)) {
		m |= HT_S_IFFIFO;
	} else if (S_ISLNK(mode)) {
		m |= HT_S_IFLNK;
	} else if (S_ISSOCK(mode)) {
		m |= HT_S_IFSOCK;
	}
	if (mode & S_IRUSR) m |= HT_S_IRUSR;
	if (mode & S_IRGRP) m |= HT_S_IRGRP;
	if (mode & S_IROTH) m |= HT_S_IROTH;

	if (mode & S_IWUSR) m |= HT_S_IWUSR;
	if (mode & S_IWGRP) m |= HT_S_IWGRP;
	if (mode & S_IWOTH) m |= HT_S_IWOTH;

	if (mode & S_IXUSR) m |= HT_S_IXUSR;
	if (mode & S_IXGRP) m |= HT_S_IXGRP;
	if (mode & S_IXOTH) m |= HT_S_IXOTH;
	return m;
}

int sys_findclose(pfind_t &pfind) {
    printf("sys_findclose()\n");
    return 0;
}

int sys_findfirst(pfind_t &pfind, const char *dirname) {
    printf("sys_findfirst(%s)\n", dirname);
    return 0;
}

int sys_findnext(pfind_t &pfind) {
    printf("sys_findnext()\n");
    return 0;
}

static void stat_to_pstat_t(const struct stat &st, pstat_t &s) {
	s.caps = pstat_ctime|pstat_mtime|pstat_atime|pstat_uid|pstat_gid|pstat_mode_all|pstat_size|pstat_inode;
	s.ctime = st.st_ctime;
	s.mtime = st.st_mtime;
	s.atime = st.st_atime;
	s.gid = st.st_uid;
	s.uid = st.st_gid;
	s.mode = sys_file_mode(st.st_mode);
	s.size = st.st_size;
	s.fsid = st.st_ino;
}

int sys_pstat(pstat_t &s, const char *filename) {
	if (!sys_filename_is_absolute(filename)) return ENOENT;
	struct stat st;
	errno = 0;
	int e = lstat(filename, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_pstat_fd(pstat_t &s, int fd) {
	struct stat st;
	errno = 0;
	int e = fstat(fd, &st);
	if (e) return errno ? errno : ENOENT;
	stat_to_pstat_t(st, s);
	return 0;
}

int sys_truncate(const char *filename, FileOfs ofs) {
    printf("sys_truncate(%s, %lld)\n", filename, ofs);
    return 0;
}

int sys_truncate_fd(int fd, FileOfs ofs) {
    printf("sys_truncate_fd(%d, %lld)\n", fd, ofs);
    return 0;
}

int sys_deletefile(const char *filename) {
    printf("sys_deletefile(%s)\n", filename);
    return 0;
}

bool sys_is_path_delim(char c) {
	return c == '/';
}

int sys_filename_cmp(const char *a, const char *b) {
    	while (*a && *b) {
		if (sys_is_path_delim(*a) && sys_is_path_delim(*b)) {
		} else if (*a != *b) {
			break;
		}
		a++;
		b++;
	}
	return *a - *b;
}

bool sys_filename_is_absolute(const char *filename) {
	return sys_is_path_delim(filename[0]);
}

SYS_FILE *sys_fopen(const char *filename, int openmode) {
    printf("sys_fopen(%s, %d)\n", filename, openmode);
    return nullptr;
}

void sys_fclose(SYS_FILE *file) {
    printf("sys_fclose()\n");
}

int sys_fread(SYS_FILE *file, byte *buf, int size) {
    printf("sys_fread(%d)\n", size);
    return 0;
}

int sys_fwrite(SYS_FILE *file, byte *buf, int size) {
    printf("sys_fwrite(%d)\n", size);
    return 0;
}

int sys_fseek(SYS_FILE *file, FileOfs newofs, int seekmode) {
    printf("sys_fseek(%lld, %d)\n", newofs, seekmode);
    return 0;
}

FileOfs sys_ftell(SYS_FILE *file) {
    printf("sys_ftell()\n");
    return 0;
}

void sys_flush(SYS_FILE *file) {
    printf("sys_flush()\n");
}

void sys_suspend() {
    EM_ASM_({ workerApi.sleep(1); });
}
