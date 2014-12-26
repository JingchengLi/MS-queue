/*
 ** This file defines necessary data structures to implement a lock-free FIFO 
 ** queue.
 **  
 ** Which is described in Michael and Scott's excellent paper appeared in PODC 
 ** '96: "Simple, Fast, and Practical Non-Blocking and Blocking Concurrent Queue 
 ** Algorithms" 
 **  
 ** Author: Jingcheng Li <jcli.china@gmail.com> 
 **  
 **/

#define __GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>

#define CAS __sync_bool_compare_and_swap

typedef int data_type;
typedef struct queue_t queue_t; 
typedef struct pointer_t pointer_t; 
typedef struct node_t node_t; 

struct node_t;

struct pointer_t {
    node_t* ptr;
    uint32_t count;   
};

struct node_t {
    data_type value;
    pointer_t next;
};

struct queue_t {
    pointer_t Head;
    pointer_t Tail;
};

void initialize(queue_t *q)
{
    node_t *node = NULL;

    node = malloc(sizeof(node_t));
    node->next.ptr = NULL;
    q->Head.ptr = q->Tail.ptr = node;
}

void enqueue(queue_t* q, data_type value){ 
    node_t *node = NULL;
    pointer_t old_tail, tail, next, tmp;

    node = malloc(sizeof(node_t));
    node->value = value;
    node->next.ptr = NULL;

    while(1)
    {
        tail = q->Tail;
        old_tail = tail;
        next = tail.ptr->next;
        /* tail may be changed in CAS after compare but before assign to q->Tail,
         * so this is incorrect:
            if (CAS((uint64_t*)&q->Tail, *(uint64_t*)&tail, *(uint64_t*)&tail))
           this is correct:
            if (CAS((uint64_t*)&q->Tail, *(uint64_t*)&tail, *(uint64_t*)&old_tail))
         */
        if (CAS((uint64_t*)&q->Tail, *(const uint64_t*)&tail, *(const uint64_t*)&tail))
        {
            if (next.ptr == NULL)
            {
                tmp.ptr = node;
                tmp.count = next.count+1;
                if (CAS((uint64_t*)&tail.ptr->next, *(const uint64_t*)&next, *(const uint64_t*)&tmp))
                {
                    break;
                }
            }
            else
            {
                tmp.ptr = next.ptr;
                tmp.count = tail.count+1;
                CAS((uint64_t*)&q->Tail, *(const uint64_t*)&tail, *(const uint64_t*)&tmp);
            }
        }
    }

    tmp.ptr = node;
    tmp.count = tail.count+1;
    CAS((uint64_t*)&q->Tail, *(const uint64_t*)&tail, *(const uint64_t*)&tmp);
}

int dequeue(queue_t *q, data_type* pvalue)
{
    pointer_t old_head, head, tail, next, tmp;
    while(1)
    {
        head = q->Head;
        old_head = head;
        tail = q->Tail;
        next = head.ptr->next;
        
        /* head may be changed in CAS after compare but before assign to q->Head,
         * so this is incorrect:
            if (CAS((uint64_t*)&q->Head, *(uint64_t*)&head, *(uint64_t*)&head))
           this is correct:
            if (CAS((uint64_t*)&q->Head, *(uint64_t*)&head, *(uint64_t*)&old_head))
         */
        if (CAS((uint64_t*)&q->Head, *(const uint64_t*)&head, *(const uint64_t*)&head))
        {
            if (head.ptr == tail.ptr)
            {
                if (next.ptr == NULL)
                {
                    return 0;
                }
                tmp.ptr = next.ptr;
                tmp.count = tail.count+1;
                CAS((uint64_t*)&q->Tail, *(const uint64_t*)&tail, *(const uint64_t*)&tmp);
            }
            else
            {
                if (pvalue)
                {
                    *pvalue = next.ptr->value;
                }
                tmp.ptr = next.ptr;
                tmp.count = head.count+1;
                if (CAS((uint64_t*)&q->Head, *(const uint64_t*)&head, *(const uint64_t*)&tmp))
                {
                    break;
                }
            }
        }
    }

    free(head.ptr);
    return 1;
}

