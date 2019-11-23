#ifndef CharBuffer_INCLUDE_GUARD
#define CharBuffer_INCLUDE_GUARD

struct CharBuffer_;
typedef struct CharBuffer_ CharBuffer;

/**
 * \fn CharBuffer_Create
 *
 * \brief
 * Create a char buffer structure that will contain
 * the character pointed by string (including \0).
 * The data from string is copied inside the structure.
 *
 * \return
 * Pointer to CharBuffer structure or 0 in case of error.
 */
CharBuffer* CharBuffer_Create(const char* string);

/**
 * \fn CharBuffer_Take
 *
 * \brief CharBuffer_Take
 * Similar with CharBuffer_Create, but takes ownership of the
 * memory region pointed by *string. The function sets string=0
 * if everything goes well.
 *
 * \return
 * Pointer to CharBuffer structure or 0 in case of error.
 */
CharBuffer* CharBuffer_Take(char** string);

/**
 * \fn CharBuffer_Append
 *
 * \brief
 * Appends the characters pointed by string into the CharBuffer structure.
 * The memory of the buffer is re-alocated to include also the characters
 * pointed by string.
 *
 * \attention
 * string must be a null terminating string
 *
 * \return
 * 1 in case of success and 0 in case of error.
 */
int CharBuffer_Append(CharBuffer* c, const char* string);

/**
 * \fn CharBuffer_Buffer
 *
 * \brief
 * Returns the internal buffer contained in the CharBufer structure.
 * Based on "option", the returned buffer can be freed by the caller or not.
 * If "option" is "GET", the caller must not free the data.
 * If "option" is "COPY", the caller must free the data.
 *
 * \return
 * Pointer to the characters contained by "c".
 */
char* CharBuffer_Buffer(CharBuffer* c, const char* option);

/**
 * \fn CharBuffer_Size
 *
 * \brief
 * Returns the number of characters inside the buffer (including \0).
 *
 * \attention
 * The function is not equivalend with strcopy.
 * To have the same behavior, you need to subtract 1.
 *
 * \return
 * The number of total characters inside "c".
 */
int CharBuffer_Size(CharBuffer* c);

/**
 * \fn  CharBuffer_Delete
 *
 * \brief
 * De-allocates the CharBuffer structure pointed by "c".
 */
void CharBuffer_Delete(CharBuffer* c);
#endif
