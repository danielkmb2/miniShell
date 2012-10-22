#include "fs.h"

/*******************************************************************************/
/** private functions to handle the shared memory segment                     **/

/* 
 * Obtains a pointer to the buffer structure that will hold the FS_buffer.
 * depending on "mode":
 *    mode == 0666|IPC_CREAT  --> tries to create the shared memory segment (shm)
 *    mode == 0666            --> tries to obtain access to the shared memory segment (shm)
 */	
t_fs *loadBuffer(int mode) {
    key_t shmkey;
    int shmid;
    void *shm_address;

    /* Generate an IPC key for the shared memory segment				   */

    shmkey = ftok(BUFFER_KEY_PATH,BUFFER_CHAR);
    if ( shmkey == -1 ){
        printf("\nmain: ftok() for shm buffer failed");
        return NULL;
    }

    shmid = shmget(shmkey, sizeof(t_fs), mode );
    if (shmid == -1){
        printf("\nmain: shmget() faileduuu\n");
        return NULL;
    }

    /* Attach the shared memory segment to the server process.       */
    shm_address = shmat(shmid, NULL, 0);
    if ( shm_address==NULL )
    {
        printf("main: shmat() failed\n");
        return NULL;
    }

    return ((t_fs *) shm_address);
}


/* obtains the shmid for the shared memory buffer for t_fs */
int getSHMID_Buffer(int mode) {
    key_t shmkey;
    int shmid;

    /* Generate an IPC key for the shared memory segment				   */

    shmkey = ftok(BUFFER_KEY_PATH,BUFFER_CHAR);
    if ( shmkey == -1 ){
        printf("main: ftok() for shm buffer failed\n");
        return -1;
    }

    shmid = shmget(shmkey, sizeof(t_fs), mode );

    return shmid;	
}


/******************************************************************************/
/* Creates the shared memory structure to hold a new filesystem.
 * If it existed previously, its data is destroyed (the new FS is initialized).
 * Returns:
 * -1  if the shared memory structure for the FS could not be created and such FS did not actually exist.
 *  0  if the filesystem could be created successfully (it did not exist previously). Its data is initialized.
 *     or  if the filesystem did already exist. Its data is destroyed.
 */
int mkFS (int maxFiles, int maxBytes) {
    t_fs *fs;

    if (maxFiles > FS_MAXFILES) {
        printf("\n unable to create a fs for more than %d files (set %d)", FS_MAXFILES , maxFiles);
        return -1;
    }

    if (maxBytes > FS_MAXBYTES) {
        printf("\n unable to create a fs with data size > %d bytes (set %d)", FS_MAXBYTES , maxBytes);
        return -1;
    }

    fs = loadBuffer(0666|IPC_CREAT);

    if (!fs) return -1;

    fs->maxFiles = maxFiles;
    fs->maxBytes = maxBytes;
    fs->numFiles = 0;
    fs->numBytes = 0;

    /* detach from the segment: */
    if (shmdt(fs) == -1) {
        perror("\n Error in shmdt(buffer)");
        exit(1);
    }	

    printf("\n MFKS completed successfully: maxbytes = %d, maxfiles = %d",maxBytes, maxFiles);

    return 0;
}


/* Destroys a shared memory segment used to hold the filesystem.
 * Returns:
 *  -1 if the shared memory did not exist previously.
 *   0 if the operation completed successfully.
 */
int dropFS() {
    int shmid = getSHMID_Buffer(0666); 
    if (shmid != -1){
        if (!shmctl(shmid, IPC_RMID, NULL))
            return 0;
    }
    return -1;
}


/* Obtains access to the existing fileSystem 
 * Returns 
 *   on success: a t_fs * pointer to a shared memory location
 *   on error: NULL
 */ 
t_fs *mountFS(){
    t_fs *fs;

    fs = loadBuffer(0666);

    return fs;
}


/* Releases access data to the existing fileSystem 
 * Returns:
 *    0 on success, and fs is set to NULL
 *   -1 if any error occurred.
 */ 
int umountFS (t_fs **fs) {

    if (shmdt(*fs) == -1) {
        perror("\n Error in shmdt(buffer)");
        return -1;
    }	
    *fs = NULL;
    return 0;
}



/* It copies the data of the file pointed by pathOfSourceFile into fs.
 * If a file named filename already existed in fs, the operation is cancelled.
 * Otherwise, a new file "filename" is created and its contents are copied from "pathOfsourceFile"
 * Returns:
 *    N: the number of bytes copied into "filename".
 *   -1:  if "filename" already existed in fs, or an error occurred.
 */
int putFS (t_fs *fs, char *pathOfSourceFile, char *filename) {
    int fd,
    	readSize,
    	i = 0,
    	tipo=0;
    struct stat fileStat;
    char *buf;
	
	if (fs == NULL) {
        printf("\n FS not mounted, fs (NULL)");
        return -1;
    }
    if (fs->numFiles >= fs->maxFiles)
    {
        printf("Sistema de ficheros lleno\n");
        return -1;
    }

    if((fd = open(pathOfSourceFile, O_RDONLY)) == -1)
        return -1;



    if(lstat(pathOfSourceFile,&fileStat)==-1)
    {
        perror("Imposible obtener los atributos del archivo\n");
    }
    else
    {
        if(S_ISREG(fileStat.st_mode)!=0) tipo=1;    /*Fichero*/
    }
    if(tipo!=1)
    {
        perror("intentando cargar algo que no es un fichero\n");
    }


    while((i < fs->maxFiles) && ((i < fs->numFiles) || (fs->entries[i].size == -1)))
        i++;

    buf = fs->data;
    buf += fs->numBytes;
    if((readSize = read(fd, buf ,fileStat.st_size)) == -1)
    { 
        perror("error al leer el fichero");
        return -1;
    }
    strncpy((char*)&(fs->entries[i].name), filename, FS_MAXFILENAME);
    fs->entries[i].size = readSize;
    fs->entries[i].pos = fs->numBytes;
    fs->numBytes += readSize;
    fs->numFiles++;
    close(fd);
    return readSize; 
}



/* It copies the data of the file "filename" from the fs into the file pathOfTargetFile.
 * If "filename" did not exist in "fs" the operation is cancelled and -1 is returned.
 * Otherwise, a the file "pathOfTargetFile" is created and the data from "filename" is 
 * copied into "pathOfTargetFile". If "pathOfTargetFile" already existed, it is overwriten.
 * Returns:
 *    N: the number of bytes copied from "filename".
 *   -1:  if "filename" did not exist in fs, or an error occurred.
 */

int buscarFS (t_fs *fs, char * nom)
{
    int i;

    for (i=0; i<fs->numFiles; i++)
        if (!strcmp(fs->entries[i].name,nom))
            return i;
    return -1;

}

int getFS (t_fs *fs, char *filename, char *pathOfTargetFile) {
    int i,
    	fd,
    	dataWrite;
    char *buf;

    if (fs == NULL) {
        printf("\n FS not mounted, fs (NULL)");
        return -1;
    }
    if ((i=buscarFS(fs,filename))==-1){
		printf("El fichero no existe en el sistema de ficheros");
        return -1;
    }
    if((fd = open(pathOfTargetFile, O_CREAT|O_RDWR,0777)) == -1)
    {
        perror("error al escribir en disco");
        return -1;
    }
    buf = fs->data;
    buf += fs->entries[i].pos;
    if ((dataWrite = write(fd, buf, fs->entries[i].size)) == -1)
    {
        perror("error al escribir el fichero\n");
        return -1;
    }

    close(fd);
    return dataWrite;
}


/* Deletes a file called "filename" from fs
 * The data in the fs is reestructured to avoid fragmentation.
 * Returns:
 *   -1: If the filename did not exist or an error occurred.
 *    0: If the operation completed successfully.
 */

int deleteFS(t_fs *fs, char *filename) {
    int i = 0, 
		j, 
		k, 
		cmp = 0,
		tam;

    if (fs == NULL) {
        printf("\n FS not mounted, fs (NULL)");
        return -1;
    }

    while((i < fs->numFiles)&&(cmp = strcmp(fs->entries[i].name,filename)))
        i++;

    if (cmp != 0){
        perror("file not found");
        return -1;
    }

    tam = fs->entries[i].size;

    for (k = fs->entries[i].pos; k<(fs->numBytes - tam); k ++){
        fs->data[k] = fs->data[k + tam];
    }


    for(j=i; j<(fs->numFiles)-1; j++){
        fs->entries[j] = fs->entries[j+1];
        fs->entries[j].pos = fs->entries[j].pos - tam;
    }
    fs->numFiles--;
	fs->numBytes=fs->numBytes-tam;

    return 0; 

}

/* It shows the information about those files within fs.
 * A line including the size, offset, and filename is shown for each file.
 * Finally, 2 additional lines summarize the statistics of the filesystems including:
 *    <number of files in fs> <max number of files in fs> < % used files>
 *    <number of bytes in fs> <max number of bytes in fs> < % used bytes>
 */
void lsFS(t_fs *fs) {
    int i;

    if (fs == NULL) {
        printf("\n FS not mounted, fs (NULL)");
        return;
    }

    printf("\n Virtual FS info:");
    printf("\n  <SIZE> <OFFSET> <NAME>");
    for (i=0;i< fs->numFiles;i++) {
        printf("\n%8d %8d %s", fs->entries[i].size,fs->entries[i].pos,fs->entries[i].name);
    }

    printf("\n\n## summary: number of files = %d, max number of files in fs = %d (%2.2f%% used)", 
            fs->numFiles,fs->maxFiles,(float)(fs->numFiles*100.0)/fs->maxFiles );
    printf("\n## summary: bytes in the fs = %d, max number of bytes in fs = %d (%2.2f%% used)", 
            fs->numBytes,fs->maxBytes,(float)(fs->numBytes*100.0)/fs->maxBytes );

}
