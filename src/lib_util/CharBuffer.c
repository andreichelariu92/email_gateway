#include <stdlib.h>
#include <string.h>

#include "MemoryRegion.h"
#include "CharBuffer.h"

struct CharBuffer_
{
    MemoryRegion* memoryRegion;
    /**
     * The number of used elements from the
     * memory region, including \0
     */
    int usedElements;
};

static inline char* toCharPtr(void* addr)
{
    return (char*)(addr);
}

static char* copyBuffer(const char* buffer)
{
    const int bufferLen = strlen(buffer) + 1;

    char* output = malloc(bufferLen * sizeof(char));
    if (output) {
        strcpy(output, buffer);
    }

    return output;
}

CharBuffer* CharBuffer_Copy(const char* string)
{
    CharBuffer* output = 0;
    int stringLen = 0;
    MemoryRegion* mr = 0;

    if (string == 0) {
        goto errorHandling;
    }

    output = malloc(sizeof(CharBuffer));
    if (output == 0) {
        goto errorHandling;
    }

    stringLen = strlen(string) + 1;
    mr = MemoryRegion_Allocate(stringLen, sizeof(char));
    if (mr == 0) {
        goto errorHandling;
    }
    memcpy(MemoryRegion_Buffer(mr), string, stringLen);

    output->memoryRegion = mr;
    output->usedElements = stringLen;

    return output;

errorHandling:
    if (output) {
        free(output);
        output = 0;
    }
    
    if (mr) {
        MemoryRegion_Delete(mr);
        mr = 0;
    }

    return output;
}

CharBuffer* CharBuffer_Take(char** string)
{
    CharBuffer* output = 0;
    MemoryRegion* mr = 0;
    int stringLen = 0;

    if (string == 0 || *string == 0) {
        goto errorHandling;
    }
    
    output = malloc(sizeof(CharBuffer));
    if (output == 0) {
        goto errorHandling;
    }
    
    stringLen = strlen(*string) + 1;
    mr = MemoryRegion_Take((void**)string, stringLen, sizeof(char));
    if (mr == 0) {
        goto errorHandling;
    }

    output->memoryRegion = mr;
    output->usedElements = stringLen;

    return output;

errorHandling:
    if (mr) {
        MemoryRegion_Delete(mr);
    }

    if (output) {
        free(output);
    }

    return 0;
}

int CharBuffer_Append(CharBuffer* c, const char* string)
{
    const int totalElements = 
        MemoryRegion_ElementCount(c->memoryRegion);
    const int stringLen = strlen(string);
    int success = (c!= 0);
    const int reallocation = 
        (totalElements - c->usedElements < stringLen);

    if (success && reallocation) {
        int newElements = (c->usedElements + stringLen) * 2;
        MemoryRegion* mr = 
            MemoryRegion_Allocate(newElements, sizeof(char));
        
        if (mr) {
            strcpy(MemoryRegion_Buffer(mr),
                   MemoryRegion_Buffer(c->memoryRegion));
            MemoryRegion_Delete(c->memoryRegion);
            c->memoryRegion = mr;
        }
        else {
            success = 0;
        }
    }
    
    if (success) {
        strcat(MemoryRegion_Buffer(c->memoryRegion), string);
        c->usedElements = c->usedElements + stringLen;
    }

    return success;
}

char* CharBuffer_Buffer(CharBuffer* c, const char* option)
{
    char* output = 0;
    
    const int COPYLEN = 4;
    const int copyOption = (strncmp(option, "copy", COPYLEN) == 0)
        || (strncmp(option, "COPY", COPYLEN) == 0);

    const int GETLEN = 3;
    const int getOption = (strncmp(option, "get", GETLEN) == 0)
        || (strncmp(option, "GET", GETLEN) == 0);

    if (c && copyOption) {
        output = copyBuffer(MemoryRegion_Buffer(c->memoryRegion));
    }
    else if (c && getOption) {
        output = MemoryRegion_Buffer(c->memoryRegion);
    }

    return output;
}

int CharBuffer_Size(CharBuffer* c)
{
    int size = (c != 0) ? c->usedElements : 0;

    return size;
}

void CharBuffer_Delete(CharBuffer* c)
{
    if (c) {
        MemoryRegion_Delete(c->memoryRegion);
        free(c);
    }
}
