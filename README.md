# Memory Allocator in C

###### Simple implementations of ***malloc()***, ***calloc()*** , ***free()***

- Uses *sbrk()* for setting program break and manipulating it
- uses a linked list to store memory blocks
- frees the memory if in middle and releases it to OS if memory block is at last

###### Compile and Run using GCC on Linux

Adjust Main function as to play around with it or export sybmols in a shared library and override default malloc provided by *stdlib.h*

*[Source Guide](https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory)*
