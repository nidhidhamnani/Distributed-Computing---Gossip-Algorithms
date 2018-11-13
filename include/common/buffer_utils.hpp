#ifndef __BUFFER_UTILS_H__
#define __BUFFER_UTILS_H__


const int max_packet_size = 65536;

// Functions to read and write int and char to void* buffer.

void increment(void **ptr, int offset) {
    *ptr = ((char*)(*ptr)) + offset;
}

void* write_char(void *ptr, char val) {
    *((char*)ptr) = val;
    return (char*)ptr + sizeof(char);
}

void* write_int(void *ptr, int val) {
    *((int*)ptr) = val;
    return (char*)ptr + sizeof(int);
}

char read_char(void **ptr) {
    char v = *((char*)(*ptr));
    *ptr = (char*)(*ptr) + sizeof(char);
    return v;
}

int read_int(void **ptr) {
    int v = *((int*)(*ptr));
    *ptr = (char*)(*ptr) + sizeof(int);
    return v;
}


#endif