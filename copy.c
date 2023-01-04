/*
Family Name: Moosa
Given Name: Sahil
Student Number: 216744344
CS Login: Sahilm1
YorkU email address (the one that appears in eClass): sahilm1@my.yorku.ca
*/

#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TEN_MILLIS_IN_NANOS 10000000

//#define DEBUG_PRINT printf
#define DEBUG_PRINT 

FILE *file_in;
FILE *file_out;
FILE *file_log;
int read_index, write_index;
int bufSize;
int readByte, writtenByte;

pthread_mutex_t lock;

typedef struct {
 char data ;
 off_t offset ;
} BufferItem ; 

BufferItem *buffer;

void *in_thread(void *param)
{
    char data ;
    off_t offset ;
    int id = (int)param; 
    struct timespec t;
    t.tv_sec = 0;
    
    while(1)
    {
        t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
        nanosleep(&t, NULL);

        pthread_mutex_lock(&lock);
        
        if((readByte - writtenByte) >= bufSize)
        {
            pthread_mutex_unlock(&lock);
            DEBUG_PRINT("Continuing because readByte:%d - writtenByte:%d >= bufSize%d\n", readByte, writtenByte, bufSize);
            continue;
        }
        //pthread_mutex_unlock(&lock);

        offset = ftell(file_in); /* get position of byte in file */
        data = fgetc(file_in); /* read byte from file */ 

        fprintf(file_log, "read_byte PT%d O%d B%d I-1 C%c %d %d\n", id, offset, data, data, readByte, writtenByte);
        DEBUG_PRINT("read_byte PT%d O%d B%d I-1 C%c %d %d\n", id, offset, data, data, readByte, writtenByte);

        t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
        nanosleep(&t, NULL);

        //pthread_mutex_lock(&lock);

        buffer[read_index].offset = offset; /* get position of byte in file */
        buffer[read_index].data = data; /* read byte from file */ 

        read_index++;
        readByte++;
        if(read_index == bufSize)
        {
            read_index = 0;
        }

        fprintf(file_log, "produce PT%d O%d B%d I%d C%c %d %d\n", id, offset, data, read_index, data, readByte, writtenByte);
        DEBUG_PRINT("produce PT%d O%d B%d I%d C%c %d %d\n", id, offset, data, read_index, data, readByte, writtenByte);

        pthread_mutex_unlock(&lock);

        if(data == EOF)
        {
            break;
        }
    }
    pthread_exit(0);
}

void *out_thread(void *param)
{
    char data ;
    off_t offset ;
    int id = (int)param; 
    struct timespec t;
    t.tv_sec = 0;
    
    while(1)
    {

        t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
        nanosleep(&t, NULL);

        pthread_mutex_lock(&lock);

        if((readByte == 0) || (writtenByte >= readByte))
        {
            pthread_mutex_unlock(&lock);
            DEBUG_PRINT("Continuing because WrittenByte:%d > readByte:%d\n", writtenByte, readByte);
            continue;
        }

        offset = buffer[write_index].offset; /* get position of byte in file */
        data = buffer[write_index].data; /* read byte from file */ 

        write_index++;
        writtenByte++;
        if(write_index == bufSize)
        {
            write_index = 0;
        }

        fprintf(file_log, "consume CT%d O%d B%d I%d C%c %d\n", id, offset, data, write_index, data,readByte, writtenByte);
        DEBUG_PRINT("consume CT%d O%d B%d I%d C%c %d\n", id, offset, data, write_index, data, readByte, writtenByte);

        if(data == EOF)
        {
            pthread_mutex_unlock(&lock);
            break;
        }


        t.tv_nsec = rand()%(TEN_MILLIS_IN_NANOS+1);
        nanosleep(&t, NULL);

        if (fseek(file_out, offset, SEEK_SET) == -1) 
        {
            fprintf(stderr, "error setting output file position to %u\n",
                (unsigned int) offset);
            exit(-1);
        }
        if (fputc(data, file_out) == EOF) 
        {
            fprintf(stderr, "error writing byte %d to output file\n", data);
            exit(-1);
        }

        fprintf(file_log, "write_byte CT%d O%d B%d I-1 C%c %d %d\n", id, offset, data, data, readByte, writtenByte);
        DEBUG_PRINT("write_byte CT%d O%d B%d I-1 C%c %d %d\n", id, offset, data, data, readByte, writtenByte);

        pthread_mutex_unlock(&lock);
                
    }
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    pthread_t *tid_in; //thread id
    pthread_attr_t *attr_in; //thread attributes

    pthread_t *tid_out; //thread id
    pthread_attr_t *attr_out; //thread attributes

    pthread_mutex_init(&lock, NULL);

    read_index = 0;
    write_index = 0;
    readByte = 0;
    writtenByte = 0;
    int nIN = 0;
    int nOUT = 0;
    char file[50];
    char copy[50];
    
    char Log[50];

    if(argc != 7)
    {
        printf("Not enough arguments passed\n");
        printf("Usage : ./copy <nIN> <nOUT> <file> <copy> <bufSize> <Log>\n");
        exit(-1);
    }
    //Parsing the input arguments

    nIN = atoi(argv[1]);
    nOUT = atoi(argv[2]);
    strcpy(file, argv[3]);
    strcpy(copy, argv[4]);
    bufSize = atoi(argv[5]);
    strcpy(Log, argv[6]);


    // printf("nIn %d, nOUT %d, file %s, copy %s, bufSize %d, Log %s\n", nIN, nOUT, file, copy, bufSize, Log);

    buffer = (BufferItem*)malloc(bufSize * sizeof(BufferItem));

    tid_in = (pthread_t*)malloc(nIN * sizeof(pthread_t));
    attr_in = (pthread_attr_t*)malloc(nIN * sizeof(pthread_attr_t));

    tid_out = (pthread_t*)malloc(nOUT * sizeof(pthread_t));
    attr_out = (pthread_attr_t*)malloc(nOUT * sizeof(pthread_attr_t));


    if (!(file_in = fopen(file, "r"))) 
    {
        perror("could not open input file for reading\n");
    }
    if (!(file_out = fopen(copy, "w+"))) 
    {
        perror("could not open output file for writing\n");
    }
     if (!(file_log = fopen(Log, "w+"))) 
    {
        perror("could not open output file for writing\n");
    }


    int k = 0;

    for(k = 0; k < nIN; k++)
    {
        pthread_attr_init(&attr_in[k]);//setting default thread attributes

        pthread_create(&tid_in[k], &attr_in[k], in_thread, (void*)k);//creating thread
    }
    for(k = 0; k < nOUT; k++)
    {
        pthread_attr_init(&attr_out[k]);//setting default thread attributes

        pthread_create(&tid_out[k], &attr_out[k], out_thread, (void*)k);//creating thread
    }

    for(k = 0; k < nIN; k++)
    {
        pthread_join(tid_in[k], NULL);
    }
    
    for(k = 0; k < nOUT; k++)
    {
        pthread_join(tid_out[k], NULL);
    }

    free(buffer);   
    free(tid_in);
    free(tid_out);
    free(attr_in);
    free(attr_out);
    fclose(file_in);
    fclose(file_out);
    fclose(file_log);
}
