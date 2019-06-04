/* This file use for NCTU OSDI course */


// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <fs.h>
#include <kernel/fs/fat/ff.h>
#include <inc/string.h>

/*TODO: Lab7, file I/O system call interface.*/
/*Note: Here you need handle the file system call from user.
 *       1. When user open a new file, you can use the fd_new() to alloc a file object(struct fs_fd)
 *       2. When user R/W or seek the file, use the fd_get() to get file object.
 *       3. After get file object call file_* functions into VFS level
 *       4. Update the file objet's position or size when user R/W or seek the file.(You can find the useful marco in ff.h)
 *       5. Remember to use fd_put() to put file object back after user R/W, seek or close the file.
 *       6. Handle the error code, for example, if user call open() but no fd slot can be use, sys_open should return -STATUS_ENOSPC.
 *
 *  Call flow example:
 *        ┌──────────────┐
 *        │     open     │
 *        └──────────────┘
 *               ↓
 *        ╔══════════════╗
 *   ==>  ║   sys_open   ║  file I/O system call interface
 *        ╚══════════════╝
 *               ↓
 *        ┌──────────────┐
 *        │  file_open   │  VFS level file API
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │   fat_open   │  fat level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    f_open    │  FAT File System Module
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │    diskio    │  low level file operator
 *        └──────────────┘
 *               ↓
 *        ┌──────────────┐
 *        │     disk     │  simple ATA disk dirver
 *        └──────────────┘
 */

extern struct fs_fd fd_table[FS_FD_MAX]; 

// Below is POSIX like I/O system call 
int sys_open(const char *file, int flags, int mode)
{
    //We dont care the mode.
	/* TODO */
	int fd = fd_new();
	if(fd == -1){	// no file descriptor
		return -1;
	}
	struct fs_fd *newfile = &fd_table[fd];
	int res = file_open(newfile, file, flags);
	if(res < 0){	// operation failed
		sys_close(fd);
		return res;
	}
	newfile->size = ((FIL*)newfile->data)->obj.objsize; // update file size
	return fd;
}

int sys_close(int fd)
{
	/* TODO */
	if(fd < 0 || fd >= FS_FD_MAX){
		return -STATUS_EINVAL;
	}

	int ret = 0;
	
	struct fs_fd *closefile = &fd_table[fd];

	if(closefile->ref_count == 1){
		closefile->size = 0;
		closefile->pos = 0;
		closefile->path[0] = '\0';
		ret = file_close(closefile);
	}

	fd_put(closefile);

	return ret;
}
int sys_read(int fd, void *buf, size_t len)
{
	/* TODO */
	if(fd < 0 || fd >= FS_FD_MAX){
		return -STATUS_EBADF;
	}
	if( buf == NULL || len < 0){
		return -STATUS_EINVAL;
	}
	if(len == 0)	return 0;

	struct fs_fd *file = fd_get(fd);
	int left = (file->size - file->pos);
	len = (len > left)? left:len;
	int ret = file_read(file, buf, len);
	fd_put(file);
	return ret;
	
}
int sys_write(int fd, const void *buf, size_t len)
{
/* TODO */
	if(fd < 0 || fd >= FS_FD_MAX){
		return -STATUS_EBADF;
	}
	if( buf == NULL || len < 0){
		return -STATUS_EINVAL;
	}
	if(len == 0)	return 0;

	struct fs_fd *file = fd_get(fd);
	int ret = file_write(file, buf, len);
	file->size = ((FIL*)file->data)->obj.objsize; // update file size
	fd_put(file);
	return ret;
}

/* Note: Check the whence parameter and calcuate the new offset value before do file_seek() */
off_t sys_lseek(int fd, off_t offset, int whence)
{
/* TODO */
	if(fd < 0 || fd >= FS_FD_MAX){
		return -STATUS_EBADF;
	}
	if(offset < 0 || whence < 0){
		return -STATUS_EINVAL;
	}

	struct fs_fd *file = fd_get(fd);
	if(whence == SEEK_END){
		offset += file->size;
	}else if(whence == SEEK_CUR){
		offset += file->pos;
	}else if(whence == SEEK_SET){
		offset = offset;
	}
	if(offset < 0){
		return -STATUS_EINVAL;
	}
	file->pos = offset;
	int ret = file_lseek(file, offset);
	fd_put(fd);

	return ret;
}

int sys_unlink(const char *pathname)
{
	/* TODO */ 
	struct fs_fd *file = NULL;
	return file_unlink(pathname);
}


              

