#ifndef STAT_H
#define STAT_H

#ifdef __MGUTILS_LIB__
#define MGUTILS_EXPORT       __declspec(dllexport)
#else
#define MGUTILS_EXPORT       __declspec(dllimport) 
#endif

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

struct stat
  {
    unsigned int/*dev_t*/ st_dev;			/* Device.  */
    unsigned int/*ino_t*/ st_ino;			/* File serial number.	*/
    unsigned int/*mode_t*/ st_mode;			/* File mode.  */
    unsigned int/*nlink_t*/ st_nlink;			/* Link count.  */
    unsigned int/*uid_t*/ st_uid;			/* User ID of the file's owner.	*/
    unsigned int/*gid_t*/ st_gid;			/* Group ID of the file's group.*/
    unsigned int/*dev_t*/ st_rdev;			/* Device number, if device.  */

    unsigned int/*off_t*/ st_size;			/* Size of file, in bytes.  */
    unsigned int/*blksize_t*/ st_blksize;		/* Optimal block size for I/O.  */

    unsigned int/*blkcnt_t*/ st_blocks;		/* Number 512-byte blocks allocated. */

	time_t st_atime;			/* Time of last access.  */
    time_t st_mtime;			/* Time of last modification.  */
    time_t st_ctime;			/* Time of last status change.  */
  };

MGUTILS_EXPORT int stat(const char* path, struct stat * buf);

#define R_OK    4       /* Test for read permission.  */
#define W_OK    2       /* Test for write permission.  */
#define X_OK    1       /* Test for execute permission.  */
#define F_OK    0       /* Test for existence.  */

MGUTILS_EXPORT int access(const char *pathname, int mode);

#endif
