#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <string.h>

#define BUFFER_KEY_PATH "/tmp"
#define BUFFER_CHAR 'B'
#define FS_MAXBYTES (1024*1024*16)
#define FS_MAXFILES 128
#define FS_MAXFILENAME  64

typedef struct {
	int size;  
	int pos;   
	char name[FS_MAXFILENAME];
} t_entry;

typedef struct {
	char data[FS_MAXBYTES];
	t_entry entries[FS_MAXFILES];
	int maxFiles;
	int maxBytes;   
	int numFiles;  
	int numBytes; 
} t_fs;

/***** FUNCTION PROTOTYPES *************/

/* Creates the shared memory structure to hold a new filesystem.
 * If it existed previously, its data is destroyed (the new FS is initialized).
 * Returns:
 * -1  if the shared memory structure for the FS could not be created and such FS did not actually exist.
 *  0  if the filesystem could be created successfully (it did not exist previously). Its data is initialized.
 *     or  if the filesystem did already exist. Its data is destroyed.
 */
int mkFS (int maxFiles, int maxBytes);


/* Destroys a shared memory segment used to hold the filesystem.
 * Returns:
 *  -1 if the shared memory did not exist previously.
 *   0 if the operation completed successfully.
 */
int dropFS();


/* Obtains access to the existing fileSystem 
 * Returns 
 *   on success: a t_fs * pointer to a shared memory location
 *   on error: NULL
 */ 
t_fs *mountFS();


/* Releases access data to the existing fileSystem 
 * Returns:
 *    0 on success, and fs is set to NULL
 *   -1 if any error occurred.
 */ 
int umountFS (t_fs **fs);



/* It copies the data of the file pointed by pathOfSourceFile into fs.
 * If a file named filename already existed in fs, the operation is cancelled.
 * Otherwise, a new file "filename" is created and its contents are copied from "pathOfsourceFile"
 * Returns:
 *    N: the number of bytes copied into "filename".
 *   -1:  if "filename" already existed in fs, or an error occurred.
 */
int putFS (t_fs *fs, char *pathOfSourceFile, char *filename);



/* It copies the data of the file "filename" from the fs into the file pathOfTargetFile.
 * If "filename" did not exist in "fs" the operation is cancelled and -1 is returned.
 * Otherwise, a the file "pathOfTargetFile" is created and the data from "filename" is 
 * copied into "pathOfTargetFile". If "pathOfTargetFile" already existed, it is overwriten.
 * Returns:
 *    N: the number of bytes copied from "filename".
 *   -1:  if "filename" did not exist in fs, or an error occurred.
 */
int getFS (t_fs *fs, char *filename, char *pathOfTargetFile);


/* Deletes a file called "filename" from fs
 * The data in the fs is reestructured to avoid fragmentation.
 * Returns:
 *   -1: If the filename did not exist.
 *    0: If the operation completed successfully.
 */

int deleteFS(t_fs *fs, char *filename);

/* It shows the information about those files within fs.
 * A line including the size, offset, and filename is shown for each file.
 * Finally, 2 additional lines summarize the statistics of the filesystems including:
 *    <number of files in fs> <max number of files in fs> < % used files>
 *    <number of bytes in fs> <max number of bytes in fs> < % used bytes>
 */
void lsFS(t_fs *fs);




