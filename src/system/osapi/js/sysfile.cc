#include <cerrno>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <emscripten.h>
#include <sys/stat.h>

#include "debug/tracers.h"
#include "system/file.h"
#include "metaimgfile.h"

typedef struct {
    MetaImgFile *disk;
    FileOfs pos;
} JSFile;

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

bool sys_is_path_delim(char c) {
	return c == '/';
}

bool sys_filename_is_absolute(const char *filename) {
	return sys_is_path_delim(filename[0]);
}

SYS_FILE *sys_fopen(const char *filename, int openmode) {
    MetaImgFile *disk = new MetaImgFile();

    std::string single_image_path;
    std::istringstream filename_stream(filename);
    while (std::getline(filename_stream, single_image_path, ':')) {
        if (!disk->open(single_image_path)) {
            delete disk;
            SYS_FILE_WARN("Failing sys_fopen(%s, %d), single_image_path=%s\n", filename, openmode, single_image_path.c_str());
            return nullptr;
        }
    }

    SYS_FILE_TRACE("sys_fopen %s -> disk ID %d\n", filename, diskId);

    return new JSFile{disk, 0};
}

void sys_fclose(SYS_FILE *file) {
    JSFile *jsFile = static_cast<JSFile*>(file);
    jsFile->disk->close();
    delete jsFile->disk;
    delete jsFile;
}

int sys_fread(SYS_FILE *file, byte *buf, int size) {
    JSFile *jsFile = static_cast<JSFile*>(file);
    uint64 readSize = jsFile->disk->read(buf, jsFile->pos, size);
    SYS_FILE_TRACE("[disk %d] sys_fread %d bytes at %d\n", jsFile->diskId, size, jsFile->pos);
    jsFile->pos += readSize;
    return readSize;
}

int sys_fwrite(SYS_FILE *file, byte *buf, int size) {
    JSFile *jsFile = static_cast<JSFile*>(file);
    uint64 writeSize = jsFile->disk->write(buf, jsFile->pos, size);
    SYS_FILE_TRACE("[disk %d] sys_fwrite %d bytes at %llu\n", jsFile->diskId, size, jsFile->pos);
    jsFile->pos += writeSize;
    return writeSize;
}

int sys_fseek(SYS_FILE *file, FileOfs newofs, int seekmode) {
    JSFile *jsFile = static_cast<JSFile*>(file);
    switch (seekmode) {
        case SYS_SEEK_SET: {
            SYS_FILE_TRACE("sys_fseek SEEK_SET to %llu\n", newofs);
            jsFile->pos = newofs;
            break;
        }
        case SYS_SEEK_REL: {
            jsFile->pos += newofs;
            SYS_FILE_TRACE("sys_fseek SEEK_CUR by %llu -> %llu\n", newofs, jsFile->pos);
            break;
        }
        case SYS_SEEK_END: {
            FileOfs size = jsFile->disk->size();
            jsFile->pos = size - newofs;
            SYS_FILE_TRACE("sys_fseek SEEK_END by %llu -> %llu\n", newofs, jsFile->pos);
            break;
        }
        default:
            SYS_FILE_TRACE("unknown sys_fseek mode %d\n", seekmode);
            break;
    }
    return 0;
}

FileOfs sys_ftell(SYS_FILE *file) {
    JSFile *jsFile = static_cast<JSFile*>(file);
    SYS_FILE_TRACE("[disk %d] sys_ftell -> %lld\n", jsFile->diskId, jsFile->pos);
    return jsFile->pos;
}

void sys_flush(SYS_FILE *file) {
    // No-op in JS
}

void sys_suspend() {
    EM_ASM_({ workerApi.sleep(1); });
}
