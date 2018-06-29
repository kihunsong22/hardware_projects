#include <time.h>
#include <stdio.h>

void * echo_thread(void* argc)
{
     (void)argc;
     int count = 0;
     struct timespec ts = { 1, 0 };
     printf("echo$ ");
     while(1)
     {
            printf("%d ", count);
            nanosleep(&ts,NULL);
            count++;
     }
     return NULL;
}




