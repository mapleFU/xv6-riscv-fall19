# Allocator

## Target

* Fast malloc and free
* Small memory overhead
* Want to use all memory
* Avoid fragmentation

## malloc algorithms

### K&R Malloc

* Using first-fit algorithm.
* coalepsce on free
* Efficient is workload-dependency:
  * If free in order, very quick
  * Otherwise, maybe search a long list.

The algorithm first prepare a list header, which contains:
* next
* size

```
// structure for free list
struct header {
    struct header *ptr;
    size_t size;
};

typedef struct header Header;

// base list
static Header base;
// the starter pointer of freelist
static Header *freep = NULL;
```

* base is the pioneer of the linked list
* freep is the start of the linked list, it will be initialized as nullptr.

There is an `moreheap` other than malloc and free, it:
* using sbrk to malloc nu (at least 1024) size of memory
* set a current header, and put it to the freelist using `free`
* return a header.

And let's goto `kr_free` and `kr_malloc`. This part you can just goto my code.

### Region Allocator




### Buddy Allocator

