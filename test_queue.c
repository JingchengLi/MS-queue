#include <stdio.h>
#include <assert.h>
#include "ms_queue.h"

pthread_t a_id[10];
pthread_t b_id[10];

queue_t queue;
void* put(void* a)
{
    int i = 0, j;
    int n = (int)a;

    for(j = n*10000000; j<(n+1)*10000000; j++)
    {
        enqueue(&queue, j);
    }
    printf("put thread: %d exit\n", n);
}

void* get(void* a)
{
    int v;
    int n = (int)a;
    int cnt = 10000000;
    while(cnt--)
    {
        while(0 == dequeue(&queue, &v))
        {
            usleep(100);
        }
    }
    printf("get thread: %d exit\n", n);
}

int main()
{
    int i, j;
    initialize(&queue);
    assert(NULL != queue.Head.ptr);
    assert(NULL != queue.Tail.ptr);
    for ( i = 0; i < 10; i++ )
    {
        pthread_create(&a_id[i], NULL, put, i);
        pthread_create(&b_id[i], NULL, get, i);
    }

    for ( i = 0; i < 10; i++ )
    {
        pthread_join(a_id[i], NULL);
        pthread_join(b_id[i], NULL);
    }

    assert(0 == dequeue(&queue, &j));
}
