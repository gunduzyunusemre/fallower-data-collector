#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t* buffer;           // Dynamic buffer pointer
    size_t size;               // Buffer size
    volatile size_t head;      // Write pointer
    volatile size_t tail;      // Read pointer
    volatile bool is_full;     // Full flag
} CircularBuffer_t;

// Core Functions
void CB_Init(CircularBuffer_t* cb, uint8_t* buffer_storage, size_t size);
void CB_Clear(CircularBuffer_t* cb);
size_t CB_GetDataSize(const CircularBuffer_t* cb);
size_t CB_GetFreeSpace(const CircularBuffer_t* cb);
bool CB_Write(CircularBuffer_t* cb, const uint8_t* data, size_t len);
size_t CB_Read(CircularBuffer_t* cb, uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // CIRCULAR_BUFFER_H