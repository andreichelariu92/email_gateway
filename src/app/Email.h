#ifndef Email_INCLUDE_GUARD
#define Email_INCLUDE_GUARD

struct _Email;
typedef struct _Email Email;

//TODO: Andrei: Add/generate documentation.
Email* Email_Create(const char* sender,
        const char* receiver,
        const char* subject,
        const char* content);

char* Email_Sender(Email* e, const char* option);
char* Email_Receiver(Email* e, const char* option);
char* Email_Subject(Email* e, const char* option);
char* Email_Content(Email* e, const char* option);
void Email_Delete(Email* e);
#endif
