#ifndef MemoryRegion_INCLUDE_GUARD
#define MemoryRegion_INCLUDE_GUARD

struct MemoryRegion_;
typedef struct MemoryRegion_ MemoryRegion;

/**
 * \fn MemoryRegion_Allocate
 *
 * \brief
 * Creates a new MemoryRegion structure and returns a pointer to
 * it. It allocates a new memory region of elementCount * elementSize
 * bytes.
 * 
 * \param elementCount
 * The number of elements.
 *
 * \param elementSize
 * The size of each element in bytes.
 *
 * \return
 * Pointer to a MemoryRegion structure or 0 in case of error.
 */
MemoryRegion* MemoryRegion_Allocate(int elementCount,
                                    int elementSize);

/**
 * \fn MemoryRegion_Take
 *
 * \brief
 * Creates a new MemoryRegion structure and returns a pointer to
 * it. It takes the ownership of the memory region pointed by *addr
 * and sets addr to 0 after the function is complete.
 *
 * \param addr
 * Pointer to a pointer that stores the begining address of the
 * memory region.
 *
 * \param elementCount
 * The number of elements.
 *
 * \param elementSize
 * The size of each element in bytes.
 *
 * \return
 * A pointer to a MemoryRegion structure or 0 in case of error.
 */
MemoryRegion* MemoryRegion_Take(void** addr, 
                                int elementCount,
                                int elementSize);

/**
 * \fn MemoryRegion_Buffer
 *
 * \brief
 * Returns the underlaying buffer inside the MemoryRegion structure.
 * If m is 0, the value returned is 0.
 */
void* MemoryRegion_Buffer(MemoryRegion* m);

/**
 * \fn MemoryRegion_ElementCount
 *
 * \brief
 * Returns the number of elements inside the MemoryRegion structure.
 * If m is 0, it returns 0.
 */
int MemoryRegion_ElementCount(MemoryRegion* m);

/**
 * \fn MemoryRegion_ElementSize
 *
 * \brief
 * Returns the size in bytes of each element stored in the
 * MemoryRegion structure.
 */
int MemoryRegion_ElementSize(MemoryRegion* m);

/**
 * \fn MemoryRegion_At
 *
 * \brief
 * Returns the address of the element at the specified index, or 0
 * if the index is out of bounds, or if m is 0.
 */
void* MemoryRegion_At(MemoryRegion* m, unsigned int idx);

/**
 * \fn MemoryRegion_Delete
 *
 * \brief
 * Cleans up all the memory used by the MemoryRegion structure,
 * including the structure itself. If m is 0, it does nothing.
 */
void MemoryRegion_Delete(MemoryRegion* m);

#endif
