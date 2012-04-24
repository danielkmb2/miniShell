/*
Daniel Padin Pazos
daniel.padin@udc.es
35484768t
grupo 3.3.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <error.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#include "valList.h"
#include "procList.h"

#define LINE_LENGTH 2048
#define MAX_ARGS (LINE_LENGTH / 2)
#define TERM 1
#define SIG 2
#define STOP 3
#define ACT 4

valListNode *valList;
procListNode *procList;

int export(int argc, char *argv[], char *line)
{
    char *clave;
    char *valor;
    char *tmp;
    valListNode *n;

    if (strstr(argv[1],"="))
    {
        if ((tmp = strtok(line, "=")))
        {
            clave = strdup(tmp);
            if ((tmp = strtok(NULL,"=")))
            {
                valor = strdup(tmp);
                if ( (n = searchValList(valList,clave)) )
                {
                    updateValList(n,valor);	
                    free(clave);
                    return 0;
                }
                else
                {
                    valList = insertValList(valList, clave, valor);
                    return 0;
                }
            }
            else
            {
                printf("Unknown command\n");
                return 0;
            }
        } 
        else
        {
            printf("Unknown command\n");
            return 0;
        }
    }
    else
    {
        printf("Unknown command\n");
        return 0;
    }
}
void vaciarListas()
{
    cleanValList(valList);
    procList = cleanProcList(procList);
}
int quit()
{
    vaciarListas();
    return 1;
}
int printAllVal()
{
    printValList(valList);
    return 0;
}
int pwd(void)
{
    char *dir;
    dir = malloc(100*sizeof(char));

    dir = getcwd(dir,100);
    printf("%s\n",dir);
    free(dir);
    return 0;
}
int pid()
{
    printf("Processs ID %d\n",getpid());	
    printf("Parent process ID %d\n",getppid());
    return 0;
}
int cd(char* argv[MAX_ARGS], int argc)
{
    if (argc==1)
        return pwd();
    else if (argc==2)
    {
        if (chdir(argv[1]))
            printf("Invalid directory\n");
        return 0;
    }
    return 0;
}
int newFork()
{
    pid_t pidHijo;

    pidHijo = fork();	
    if (pidHijo==0)
        return 0;
    else
    {
        waitpid(pidHijo,NULL,0);
        return 0;
    }
}
int execute(char* argv[MAX_ARGS],int argc)
{
    execvp(argv[0],&argv[0]);
    printf("execvp: %s\n",strerror(errno));
    vaciarListas();
    exit(1);
    return 1;
}
int executeInFront(char* argv[MAX_ARGS],int argc)
{
    pid_t pidHijo;

    pidHijo = fork();	
    if (pidHijo==0)
    {
        execute(argv,argc);
        exit(1);
        return 1;
    }
    else
    {
        waitpid(pidHijo,NULL,0);
        return 0;
    }	
}
int background(char* argv[MAX_ARGS])
{
    pid_t pidHijo;

    pidHijo = fork();	
    if (pidHijo==0)
    {
        execvp(argv[1],&argv[1]);
        printf("execvp: %s\n",strerror(errno));
        vaciarListas();
        exit(1);
        return 1;
    }
    else
    {
        char *nombre;
        nombre = strdup(argv[1]);
        procList = addProcList(procList, nombre, pidHijo, 0);
        return 0;
    }
}
procListNode* obtainPidInfo(int stat_loc,procListNode* tmpNode)
{
    if (getpriority(PRIO_PROCESS,tmpNode->pidHijo) != -1)
        tmpNode->priority = getpriority(PRIO_PROCESS,tmpNode->pidHijo);

    if (waitpid(tmpNode->pidHijo,&stat_loc,WNOHANG | WCONTINUED | WUNTRACED) > 0)
    {
        if (WIFEXITED(stat_loc))
        {
            tmpNode->status = TERM;
            tmpNode->returned = WEXITSTATUS(stat_loc);
        }
        if (WIFSIGNALED(stat_loc))
        {
            tmpNode->status = SIG;
            tmpNode->returned = WTERMSIG(stat_loc);
        }
        if (WIFSTOPPED(stat_loc))
            tmpNode->status = STOP;
        if (WIFCONTINUED(stat_loc))
            tmpNode->status = ACT;
    }
    return tmpNode;
}
char *getStringStatus(int status)
{
    /*allocs stringStatus!!*/
    if(status==TERM)
        return strdup("terminated");
    if(status==SIG)
        return strdup("signaled");
    if(status==STOP)
        return strdup("stopped");
    if(status==ACT)
        return strdup("running");

    return strdup("ERROR!");/*to avoid warnings*/
}
int jobOps(char* argv[MAX_ARGS],int argc)
{
    procListNode *tmpNode, *next;
    char* stringStatus;
    int stat_loc = 0;

    tmpNode = procList;
    while(tmpNode)
    {
        tmpNode = obtainPidInfo(stat_loc,tmpNode);
        tmpNode = tmpNode->sig;
    }
    tmpNode = procList;
    while(tmpNode)
    {
        stringStatus = getStringStatus(tmpNode->status);
        if ((!strcmp(argv[1],"all"))||
           ((!strcmp(argv[1],"term"))&&(tmpNode->status==TERM))||
           ((!strcmp(argv[1],"sig"))&&(tmpNode->status==SIG))||
           ((!strcmp(argv[1],"stop"))&&(tmpNode->status==STOP))||
           ((!strcmp(argv[1],"act"))&&(tmpNode->status==ACT)))
                printf("%s, pid: %d, priority: %d, %s, returned: %d at %s",
                    tmpNode->path,tmpNode->pidHijo,tmpNode->priority,stringStatus,
                    tmpNode->returned,ctime(&tmpNode->creation));

        next = tmpNode->sig;
        if (!strcmp(argv[1],"-del"))
        {
            if ((!strcmp(argv[2],"all"))||
               ((!strcmp(argv[2],"term"))&&(tmpNode->status==TERM))||
               ((!strcmp(argv[2],"sig"))&&(tmpNode->status==SIG))||
               ((!strcmp(argv[2],"stop"))&&(tmpNode->status==STOP))||
               ((!strcmp(argv[2],"act"))&&(tmpNode->status==ACT)))
                   procList = delProcList(procList,tmpNode->pidHijo);
         }
        free(stringStatus);
        tmpNode = next;
    }
    return 0;	
}
void st_modeToString(mode_t st_mode)
{
		if(S_ISDIR(st_mode)!=0) printf("d"); else printf("-");
    	if((st_mode & S_IRUSR)!=0) printf("r"); else printf("-");
		if((st_mode & S_IWUSR)!=0) printf("w"); else printf("-");
		if((st_mode & S_IXUSR)!=0) printf("x"); else printf("-");
		if((st_mode & S_IRGRP)!=0) printf("r"); else printf("-");
		if((st_mode & S_IWGRP)!=0) printf("w"); else printf("-");
		if((st_mode & S_IXGRP)!=0) printf("x"); else printf("-");
		if((st_mode & S_IROTH)!=0) printf("r"); else printf("-");
		if((st_mode & S_IWOTH)!=0) printf("w"); else printf("-");
		if((st_mode & S_IXOTH)!=0) printf("x"); else printf("-");
}
void showFileInfo(struct stat fileStat, char *path,char *name,int extended)
{
    struct tm *t;
    struct group* grupo;
    struct passwd* user;
    int tipo;
    char linkName[1024];

    lstat(path,&fileStat);
    if ((grupo = getgrent()))
        while(grupo->gr_gid != fileStat.st_gid) 
            grupo = getgrent();
    endgrent();
    user = getpwuid(fileStat.st_uid);

    if(lstat(path,&fileStat)==-1){
        printf("Imposible obtener los atributos del archivo: %s\n", path);
    }
    else{
        if(S_ISREG(fileStat.st_mode)!=0) tipo=1;    /*Fichero*/
        if(S_ISDIR(fileStat.st_mode)!=0) tipo=2;    /*Directorio*/
        if(S_ISLNK(fileStat.st_mode)!=0) tipo=3;    /*Enlace simbolico*/
    }
    if(tipo==3)
    {
        char buf[PATH_MAX];
        realpath(path, buf);
        snprintf(linkName,1024,"%s -> %s",name,buf);
    }
    else
        snprintf(linkName,1024,"%s",name);
    if(extended)
    {    
        printf("%li\t%li\t",fileStat.st_ino,fileStat.st_blocks);
        st_modeToString(fileStat.st_mode);
        t=gmtime(&fileStat.st_ctime);
        printf("\t%d\t%s\t%s\t%li\t%d-%d-%d\t%d:%d\t%s\n",
                (int)fileStat.st_nlink,user->pw_name,grupo->gr_name,
                fileStat.st_size,t->tm_year+1900,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min ,linkName);
    } 
    else
        if(strncmp(name,".",1))

        {
            printf("%li\t%li\t",fileStat.st_ino,fileStat.st_blocks);
            st_modeToString(fileStat.st_mode);
            t=gmtime(&fileStat.st_ctime);
            printf("\t%d\t%s\t%s\t%li\t%d-%d-%d\t%d:%d\t%s\n",
                    (int)fileStat.st_nlink,user->pw_name,grupo->gr_name,
                    fileStat.st_size,t->tm_year+1900,t->tm_mon,t->tm_mday,t->tm_hour,t->tm_min,linkName);
        }
}
int ls(char* argv[MAX_ARGS],int argc)
{
    DIR *dirp;
    struct dirent* direntp;
    char *path;
    char aux[1024];
    int tipo;
    struct stat fileStat;

    if((!strcmp(argv[argc-1],"-h"))||(!strcmp(argv[argc-1],"-l"))||(!strcmp(argv[argc-1],"LS")))
        path=strdup(".");
    else
        path=strdup(argv[argc-1]);

    if(lstat(path,&fileStat)==-1){
        printf("Imposible obtener los atributos del archivo: %s\n", path);
        free(path);
        return(0);
    }
    else{
        if(S_ISREG(fileStat.st_mode)!=0) tipo=1;    /*Fichero*/
        if(S_ISDIR(fileStat.st_mode)!=0) tipo=2;    /*Directorio*/
        if(S_ISLNK(fileStat.st_mode)!=0) tipo=3;    /*Enlace simbolico*/
    }

    if(tipo==2)
    {
        dirp = opendir(path);
        if (dirp == NULL){
            printf("Error: No se puede abrir el directorio\n");
            free(path);
            return 0;
        }

        while ((direntp = readdir(dirp)) != NULL) 
        {
            if((argc>1)&&(!strcmp(argv[1],"-h")))
            {
                printf("%s\n",direntp->d_name);
            }
            else
                if((argc>2)&&(!strcmp(argv[1],"-l"))&&(!strcmp(argv[2],"-h")))
                {
                    snprintf(aux,1024,"%s/%s",path,direntp->d_name);
                    showFileInfo(fileStat,aux,direntp->d_name,1);
                }
                else
                    if((argc>1)&&(!strcmp(argv[1],"-l")))
                    {
                        snprintf(aux,1024,"%s/%s",path,direntp->d_name);
                        showFileInfo(fileStat,aux,direntp->d_name,0);
                    }
                    else
                    {
                        if(strncmp(direntp->d_name,".",1))
                            printf("%s\n",direntp->d_name);
                    }
        }
        closedir(dirp);
    }

    if(tipo==1)
    {
        if((argc>1)&&(!strcmp(argv[1],"-l")))
        {
            showFileInfo(fileStat,path,path,0);
        }
        else
        {
            if(strncmp(path,".",1))
                printf("%s\n",path);
        }
    }

    if(tipo==3)
    {
        char buf[PATH_MAX];
        realpath(path, buf);
        argv[argc-1]=buf;
        ls(argv,argc);
    }
    free(path);
    return 0;
}
int parse(char *line)
{
    char *argv[MAX_ARGS];
    int argc = 0;

    if(line[0]==' ')
    {
        printf("Unknown command\n");
        return 0;
    }

    if(!(argv[argc] = strtok(line," \t\n")))
        return 0;
    argc++;
    while((argv[argc] = strtok(NULL, " \t\n")))
        argc++;

    if ((!strcmp(argv[0],"quit"))&&(argc==1))		/* vaciamos listas y salimos */
        return quit();
    if ((!strcmp(argv[0],"export"))&&(argc==1))		/* muestra todas las variables */
        return printAllVal();
    if ((!strcmp(argv[0],"export"))&&(argc==2))		/* asignaciones de variables */
        return export(argc,argv,argv[1]);
    if ((!strcmp(argv[0],"pid"))&&(argc==1))		/* devolver el pid de la shell */
        return pid();
    if ((!strcmp(argv[0],"pwd"))&&(argc==1))		/* mostrar directorio actual */
        return pwd();
    if (!strcmp(argv[0],"cd"))						/* cambiar directorio o pwd */
        return cd(argv,argc);
    if (!strcmp(argv[0],"LS"))                      /* listar directorios */
        return ls(argv,argc);
    if ((!strcmp(argv[0],"fork"))&&(argc==1))		/* crear nuevo proceso */
        return newFork();
    if ((!strcmp(argv[0],"execute"))&&(argc>=2))	/* sustituír por código de prog */
        return execute(&(argv[1]),argc-1);
    if ((!strcmp(argv[0],"background"))&&(argc>=2))	/* ejecutar en 2º plano */
        return background(argv);
    if ((!strcmp(argv[0],"jobs"))&&(argc>=2))		/* información sobre los proc 2º plano */
        return jobOps(argv,argc);
    if (argc>=1)									/* "execute" y devolver control */
        return executeInFront(argv,argc);

    printf("Unknown command\n");	
    return 0;
}

int main(int argc, char *argv[], char *env[])
{
    int q=0;
    char *l;

    l = malloc(LINE_LENGTH*sizeof(char));

    valList = createValList();
    procList = createProcList();

    while(!q)
    {
        printf("# ");
        fgets(l, LINE_LENGTH, stdin);
        q = parse(l);
    }
    free(l);	
    return 0;
}
