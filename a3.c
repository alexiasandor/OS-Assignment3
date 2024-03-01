#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define PIPE_NAME_1 "RESP_PIPE_56939"

int main (){
int fd_resp =-1;
int fd_req=-1;


// verificam crearea  pipe-ul
if(mkfifo(PIPE_NAME_1, 0600) != 0){
    perror("ERROR\n");
    perror("cannot create the respone pipe");
    return 1;
}

//deschidem in citire pipe-ul creat de tester
fd_req=open("REQ_PIPE_56939", O_RDONLY);
if(fd_req==-1){
     perror("ERROR\n");
    perror("cannot open the request pipe");
    return 1;
}

//VERIFICAM  deschiderea pipe-ului in scriere
fd_resp= open(PIPE_NAME_1, O_WRONLY);
if(fd_resp==-1){
    perror("ERROR\n");
    perror("cannot create the respone pipe");
    return 1;
}

write(fd_resp,"CONNECT#",8);
printf("SUCCESS\n");
char s1[100];
char c=0;
int nr_varianta=56939;
char s2[100];
char c1=0;
int poz1=0;
//pentru cerinta 2
unsigned int mem_partajata=0;
int fd_mem_partajata=-1;
//pentru cerinta 6
volatile char*data =NULL;
volatile char*data1=NULL;
int fd_fisier = -1;
int size=0;
//pentru cerinta 7
unsigned int no_of_bytes=0;
unsigned int offset=0;
int i=0;

while(1){
    int poz=0;
    c=0;
    while(c!='#'){
    // citim caracter cu caracter din pipe ul generat de tester 
    read(fd_req,&c,1);
    //salvam undeva 
    s1[poz]=c;
    poz++;
    }
    s1[poz-1]='\0';

    //verificam daca ce am citit este "PING", 2.3
    if(strncmp(s1,"PING",4) == 0){
        write(fd_resp,"PING#",5);
        write(fd_resp,&nr_varianta,4);
        write(fd_resp,"PONG#",5);
    }
    //verificam 2.4 cu CREATE_SHM
    if(strncmp(s1,"CREATE_SHM",10) == 0){
        
            read(fd_req, &mem_partajata,sizeof(mem_partajata));
        
        fd_mem_partajata=shm_open("/1pPjgCM", O_CREAT | O_RDWR, 0644);

        if(fd_mem_partajata == -1){
            write(fd_resp, "CREATE_SHM#",11);
            write(fd_resp, "ERROR#",6);
        }
        else {
            data = (volatile char*)mmap(NULL,mem_partajata,PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem_partajata,0); 
            if(data == (void*)-1){
            write(fd_resp, "CREATE_SHM#",11);
            write(fd_resp, "ERROR#",6);
            }
            else {
                write(fd_resp, "CREATE_SHM#",11);
                write(fd_resp, "SUCCESS#",8);
            }
        }
    }
    //2.5
    if(strncmp(s1, "WRITE_TO_SHM",12) == 0){
        break;
    }
    //verificam 2.6 MAP
    if(strncmp(s1, "MAP_FILE",8) == 0){
        while(c1!='#'){
        // citim caracter cu caracter din pipe ul generat de tester 
        read(fd_req,&c1,1);
        //salvam undeva 
        s2[poz1]=c1;
        poz1++;
        }
        s2[poz1-1]='\0';
        //declaram fisierul din care se citeste 
        fd_fisier = open(s2, O_RDONLY);
        if(fd_fisier == -1){
            write(fd_resp, "MAP_FILE#",9);
            write(fd_resp,"ERROR#",6);
        }
        else{ 
        size = lseek(fd_fisier,0,SEEK_END);
        lseek(fd_fisier,0,SEEK_SET);

        
        data1=(volatile char*)mmap(NULL,size,PROT_READ,MAP_SHARED,fd_fisier,0);
        if(data1 == (void*)-1){
            write(fd_resp, "MAP_FILE#",9);
            write(fd_resp,"ERROR#",6);
        }
        else{
        write(fd_resp,"MAP_FILE#",9);
        write(fd_resp,"SUCCESS#",8);}

        }
    }
    //verificam 2.7
    if(strncmp(s1, "READ_FROM_FILE_OFFSET",21) == 0){
        //citim din fisierul mapat
        read(fd_req,&offset,sizeof(offset));
        read(fd_req,&no_of_bytes,sizeof(no_of_bytes));
        // verificam daca exista o regiune de memorie partajata
        if(data == (void*)-1 || data1 == (void*)-1){
         write(fd_resp, "READ_FROM_FILE_OFFSET#",22);
         write(fd_resp, "ERROR#",6);
        }
        else
        {
            //verificam daca un fisier este mapat in memorie
            if(fd_mem_partajata < 0){
                write(fd_resp, "READ_FROM_FILE_OFFSET#",22);
                write(fd_resp,"ERROR#",6);
            }
            else
            {
                //verificam daca offestul cerut+nr_octeti<dim fisier;
                if(offset+no_of_bytes<size)
                {
                    for(i = 0;i<no_of_bytes;i++)
                    {
                        data[i]=data1[offset];
                        offset++;
                    }
                    write(fd_resp, "READ_FROM_FILE_OFFSET#",22);
                    write(fd_resp, "SUCCESS#", 8);
                }
                else{
                    write(fd_resp, "READ_FROM_FILE_OFFSET#",22);
                    write(fd_resp, "ERROR#", 6);
                }       
            }
        }
    }
   //2.8
   if(strncmp(s1,"READ_FROM_FILE_SECTION",22) == 0){
    break;
   }
   //2.9
   if(strncmp(s1,"READ_FROM_LOGICAL_SPACE_OFFSET",30) == 0){
    break;
   }
   //2.10
   if(strncmp(s1,"EXIT",4) == 0){
    break;
    }
}
//inchidem
close(fd_mem_partajata);
close(fd_fisier);
close(fd_req);
close(fd_resp);
unlink(PIPE_NAME_1);
return 0;
}