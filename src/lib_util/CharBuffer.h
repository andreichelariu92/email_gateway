#ifndef CharBuffer_INCLUDE_GUARD
#define CharBuffer_INCLUDE_GUARD

struct CharBuffer_;
typedef struct CharBuffer_ CharBuffer;

//TODO: Andrei: Add/generate doxygen comments.

CharBuffer* CharBuffer_Copy(const char* string);

CharBuffer* CharBuffer_Take(char** string);

int CharBuffer_Append(CharBuffer* c, const char* string);

char* CharBuffer_Buffer(CharBuffer* c, const char* option);

int CharBuffer_Size(CharBuffer* c);

void CharBuffer_Delete(CharBuffer* c);
#endif
