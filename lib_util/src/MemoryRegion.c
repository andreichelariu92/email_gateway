#include "MemoryRegion.h"

#include <stdlib.h>

struct MemoryRegion_
{
    void* addr;
    int elementCount;
    int elementSize;
};

MemoryRegion* MemoryRegion_Allocate(int elementCount, 
                                    int elementSize)
{
    MemoryRegion* output = 0;

    if (elementCount <= 0 || elementSize <= 0) {
        goto errorHandling;
    }

    output = malloc(sizeof(MemoryRegion));
    if (!output) {
        goto errorHandling;
    }
    
    output->addr = malloc(elementCount * elementSize);
    if (!output->addr) {
        goto errorHandling;
    }

    output->elementCount = elementCount;
    output->elementSize = elementSize;

    return output;

errorHandling:
    if (output && output->addr) {
        free(output->addr);
        free(output);
        output = 0;
    }
    else if (output) {
        free(output);
        output = 0;
    }

    return output;
}

MemoryRegion* MemoryRegion_Take(void** addr,
                                int elementCount,
                                int elementSize)
{
    MemoryRegion* output = 0;
    
    /*validate input parameters*/
    if (addr == 0 || (*addr) == 0) {
        goto errorHandling;
    }
    if (elementCount <= 0 || elementSize <= 0) {
        goto errorHandling;
    }

    output = malloc(sizeof(MemoryRegion));
    if (output == 0) {
        goto errorHandling;
    }
    
    output->addr = *addr;
    /*do not allow the client code to use the buffer
     *outside the MemoryRegion structure*/
    *addr = 0;
    output->elementCount = elementCount;
    output->elementSize = elementSize;

    return output;

errorHandling:
    /* there is no need for a deallocation */
    return output;
}

void* MemoryRegion_Buffer(MemoryRegion* m)
{
    if (m) {
        return m->addr;
    }
    else {
        return 0;
    }
}

int MemoryRegion_ElementCount(MemoryRegion* m)
{
    if (m) {
        return m->elementCount;
    }
    else {
        return 0;
    }
}

int MemoryRegion_ElementSize(MemoryRegion* m)
{
    if (m) {
        return m->elementSize;
    }
    else {
        return 0;
    }
}

void* MemoryRegion_At(MemoryRegion* m, unsigned int idx)
{
    void* output = 0;
    
    if (m && (idx < m->elementCount)) {
        output = m->addr + (idx * m->elementSize);
    }

    return output;
}

void MemoryRegion_Delete(MemoryRegion* m)
{
    if (m) {
        free(m->addr);

        m->elementCount = 0;
        m->elementSize = 0;

        free(m);
    }
}
