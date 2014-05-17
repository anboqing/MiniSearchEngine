#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

void *thread_func(void *arg);

int main(int argc, char const *argv[])
{

    char buf[1024];

    pthread_t tid;

    pthread_create(&tid, NULL, thread_func, &buf);
    int i = 0;
    while(1){
    	buf[i++] = 'a'+i;
    	sleep(1);
    }

    return 0;
}

void *thread_func(void *arg)
{
	char *buf = (char*)arg;
	while(1){
		if(strlen(buf)!=0){
			cout << buf << endl;
		}
		sleep(1);
	}
}