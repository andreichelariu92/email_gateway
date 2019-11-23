#include "lib_util/src/CharBuffer.h"
#include "Email.h"

#include <string.h>
#include <stdlib.h>

struct _Email
{
    CharBuffer* sender;
    CharBuffer* receiver;
    CharBuffer* subject;
    CharBuffer* content;
};

static int isValidEmail(const char* email)
{
    char* tokenPosition  = 0;

    tokenPosition = strchr(email, '@');
    
    return (tokenPosition != NULL);
}

Email* Email_Create(const char* sender,
        const char* receiver,
        const char* subject,
        const char* content)
{
    int validInput = 0;
    int allocationSuccess = 0;
    
    CharBuffer* senderBuff = 0;
    CharBuffer* receiverBuff = 0;
    CharBuffer* subjectBuff = 0;
    CharBuffer* contentBuff = 0;

    Email* output = 0;

    validInput = (sender != NULL) && (receiver != NULL)
        && (subject != NULL) && (content != NULL);

    validInput = validInput ? isValidEmail(sender) : validInput;
    validInput = validInput ? isValidEmail(receiver) : validInput;

    senderBuff = validInput ? CharBuffer_Create(sender) : NULL;
    allocationSuccess = (senderBuff != NULL);

    receiverBuff = (validInput && allocationSuccess) 
        ? CharBuffer_Create(receiver)
        : NULL;
    allocationSuccess = (receiverBuff != NULL);

    subjectBuff = (validInput && allocationSuccess)
        ? CharBuffer_Create(subject)
        : NULL;
    allocationSuccess = (subjectBuff != NULL);

    contentBuff = (validInput && allocationSuccess)
        ? CharBuffer_Create(content)
        : NULL;
    allocationSuccess = (contentBuff != NULL);

    output = (validInput && allocationSuccess)
        ? malloc (sizeof(Email))
        : NULL;
    allocationSuccess = (output != NULL);

    if (validInput && allocationSuccess) {
        output->sender = senderBuff;
        output->receiver = receiverBuff;
        output->subject = subjectBuff;
        output->content = contentBuff;
    }
    else {
        CharBuffer_Delete(senderBuff);
        CharBuffer_Delete(receiverBuff);
        CharBuffer_Delete(subjectBuff);
        CharBuffer_Delete(contentBuff);

        free(output);
        output = NULL;
    }

    return output;
}

char* Email_GetSender(Email* e, const char* option)
{
    char* output = NULL;

    if (e) {
        output = CharBuffer_Buffer(e->sender, option);
    }

    return output;
}

char* Email_GetReceiver(Email* e, const char* option)
{
    char* output = NULL;

    if (e) {
        output = CharBuffer_Buffer(e->receiver, option);
    }

    return output;
}

char* Email_GetSubject(Email* e, const char* option)
{
    char* output = NULL;

    if (e) {
        output = CharBuffer_Buffer(e->subject, option);
    }

    return output;
}

char* Email_GetContent(Email* e, const char* option)
{
    char* output = NULL;

    if (e) {
        output = CharBuffer_Buffer(e->content, option);
    }

    return output;
}

void Email_Delete(Email* e)
{
    if (e) {
        CharBuffer_Delete(e->sender);
        CharBuffer_Delete(e->receiver);
        CharBuffer_Delete(e->subject);
        CharBuffer_Delete(e->content);

        free(e);
    }
}
