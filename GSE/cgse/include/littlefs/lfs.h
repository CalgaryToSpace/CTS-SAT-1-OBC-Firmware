#ifndef LFS_H
#define LFS_H

typedef uint32_t lfs_size_t;
typedef uint32_t lfs_off_t;

typedef int32_t  lfs_ssize_t;
typedef int32_t  lfs_soff_t;

typedef uint32_t lfs_block_t;

#ifndef LFS_NAME_MAX
#define LFS_NAME_MAX 255
#endif

// File open flags
enum lfs_open_flags {
    // open flags
    LFS_O_RDONLY = 1,         // Open a file as read only
#ifndef LFS_READONLY
    LFS_O_WRONLY = 2,         // Open a file as write only
    LFS_O_RDWR   = 3,         // Open a file as read and write
    LFS_O_CREAT  = 0x0100,    // Create a file if it does not exist
    LFS_O_EXCL   = 0x0200,    // Fail if a file already exists
    LFS_O_TRUNC  = 0x0400,    // Truncate the existing file to zero size
    LFS_O_APPEND = 0x0800,    // Move to end of file on every write
#endif

    // internally used flags
#ifndef LFS_READONLY
    LFS_F_DIRTY   = 0x010000, // File does not match storage
    LFS_F_WRITING = 0x020000, // File has been written since last flush
#endif
    LFS_F_READING = 0x040000, // File has been read since last flush
#ifndef LFS_READONLY
    LFS_F_ERRED   = 0x080000, // An error occurred during write
#endif
    LFS_F_INLINE  = 0x100000, // Currently inlined in directory entry
};

// File seek flags
enum lfs_whence_flags {
    LFS_SEEK_SET = 0,   // Seek relative to an absolute position
    LFS_SEEK_CUR = 1,   // Seek relative to the current file position
    LFS_SEEK_END = 2,   // Seek relative to the end of the file
};


/// General operations ///

typedef int lfs_t;
struct lfs_info {};
typedef int lfs_file_t;

int lfs_remove(lfs_t *lfs, const char *path) { return 0; }

int lfs_rename(lfs_t *lfs, const char *oldpath, const char *newpath) { return 0; }

int lfs_stat(lfs_t *lfs, const char *path, struct lfs_info *info) { return 0; }

int lfs_file_open(lfs_t *lfs, lfs_file_t *file, const char *path, int flags) { return 0; }

int lfs_file_close(lfs_t *lfs, lfs_file_t *file) { return 0; }

int lfs_file_sync(lfs_t *lfs, lfs_file_t *file) { return 0; }

lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file, void *buffer, lfs_size_t size) { return 0; }

lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file, const void *buffer, lfs_size_t size) { return 0; }

lfs_soff_t lfs_file_seek(lfs_t *lfs, lfs_file_t *file, lfs_soff_t off, int whence) { return 0; }

int lfs_file_truncate(lfs_t *lfs, lfs_file_t *file, lfs_off_t size) { return 0; }

lfs_soff_t lfs_file_tell(lfs_t *lfs, lfs_file_t *file) { return 0; }

int lfs_file_rewind(lfs_t *lfs, lfs_file_t *file) { return 0; }

lfs_soff_t lfs_file_size(lfs_t *lfs, lfs_file_t *file) { return 0; }

#endif
