#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
/**
 * Gaal: Write a Memory Allocator
 * To implement : malloc(), calloc(), realloc(), free()
 *
 * What is Needed?
 * Memory layout of a program
 * Text Section, Data Section, BSS, Heap, Stack
 * On Unix use a syscall brk, sbrk (use command "man brk")
 *
 */

/*
 * alignment variable since union stores occupies the size of the biggest variable
 * It makes our header_t memory aligned using 16 bytes
 */
typedef char ALIGN[16];

union header
{
    // a header for each block
    struct
    {
        size_t size;          // block size
        unsigned int is_free; // is block free?
        union header *next;   // keep track of the next block (like a linked list)
    } s;
    ALIGN stub; // for memory alignment
};

typedef union header header_t;

// head and tail pointers for keeping track of memory
header_t *head, *tail;

// locking mechanism for preventing simultaneous memory access
pthread_mutex_t global_malloc_lock;

/**
 * get free block in memory of sufficient size using first fit searching basis on the linked list
 * @param size the size of memory block needed
 * @returns NULL if no block of memory free Else  pointer to the block of memory free if such exists
 */
header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while (curr)
    {
        if (curr->s.is_free && curr->s.size >= size)
        {
            return curr;
        }
        curr = curr->s.next;
    }
    return NULL;
}

// Easiest Function if freeing is not involved  :()
void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
    if (!size)
    { // if size 0 return null
        return NULL;
    }
    pthread_mutex_lock(&global_malloc_lock); // lock for current thread
    header = get_free_block(size);
    if (header)
    {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void *)(header + 1); // header is 16byte adding 1 to 16byte address
    }
    total_size = size + sizeof(header);
    block = sbrk(total_size);
    if (block == (void *)-1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = (header_t*)block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if (!head)
    {
        head = header;
    }
    if (tail)
    {
        tail->s.next = header;
    }
    pthread_mutex_unlock(&global_malloc_lock);
    return (void *)(header + 1);
}

void free(void *block)
{
    header_t *header, *tmp;
    void *programbreak;

    if (!block)
        return;
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t *)block - 1;

    programbreak = sbrk(0);
    // if block at the end of programbrk release it to os
    if ((char *)block + header->s.size == programbreak)
    {
        if (head == tail)
        {
            head = tail = NULL;
        }
        else
        {
            tmp = head;
            while (tmp)
            {
                if (tmp->s.next == tail)
                {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        // free the memory
        sbrk(0 - sizeof(header_t) - header->s.size);
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    // if in middle just free it and return
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

/**
 * Like malloc(n * size) but checks for overflow
 */
void *calloc(size_t num, size_t nsize)
{
    size_t size;
    void *block;
    if (!num || !nsize)
    {
        return NULL;
    }
    size = num * nsize;
    /*check multiplication overflow*/
    if (nsize != size / num)
    {
        return NULL;
    }
    block = malloc(size);
    if (!block)
    {
        return NULL;
    }
    memset(block, 0, size);
    return block;
}

/**
 * Realloc the current block to new_size
 */
void *realloc(void *block, size_t size)
{
    header_t *header;
    void *ret;
    if (!block || !size)
    {
        return malloc(size);
    }
    header = (header_t *)block - 1;
    if (header->s.size >= size)
    {
        return block;
    }
    ret = malloc(size);
    if (ret)
    {
        memcpy(ret, block, header->s.size);
        free(block);
    }
    return ret;
}
int main()
{
    // Test malloc
    int N = 10;
    int *arr = (int*) malloc(10);
    for (int i = 0; i < 10; i++)
    {
        arr[i] = i;
    }
    for (int i = 0; i < 10; i++)
    {
        printf(" %d ,", i);
    }
    int *new_arr = (int*) calloc(10, 18);
    free(arr);
    free(new_arr);
    int x = arr[0];
    printf("new val %d ", x);
    printf("\nEnd of Program\n");
    return 0;
}
