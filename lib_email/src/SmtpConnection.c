#include "SmtpConnection.h"
#include "lib_util/src/CharBuffer.h"
#include "lib_util/src/MemoryRegion.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

const int SMTP_SSL = 0;
const int SMTP_NO_SSL = 1;

struct _SmtpConnection
{
    CURL* handle;
    CharBuffer* addr;
    int port;
    int isSsl;
    CharBuffer* user;
    CharBuffer* pass;
    Email* currentEmail;
    int emailSent;
};

static int firstConnection = 1;

struct curl_slist* makeReceiverList(const char* name)
{
    struct curl_slist* output = NULL;
    output = curl_slist_append(output, name);

    return output;
}

static CharBuffer* formatField(const char* prefix,
    const char* field,
    const char* sufix)
{
    CharBuffer* output = NULL;
    int formatSuccess = 0;

    formatSuccess = 
        (prefix != NULL && field != NULL && sufix != NULL);

    output = formatSuccess ? CharBuffer_Create(prefix) : NULL;
    formatSuccess = (output != NULL);
    
    formatSuccess = 
        formatSuccess ? CharBuffer_Append(output, field) : 0;

    formatSuccess =
        formatSuccess ? CharBuffer_Append(output, sufix) : 0;

    if (!formatSuccess) {
        CharBuffer_Delete(output);
        output = NULL;
    }

    return output;
}

static CharBuffer* formatEmail(Email* e)
{
    CharBuffer* output = NULL;
    int formatSuccess = 0;
    char* sender = NULL;
    char* receiver = NULL;
    char* subject = NULL;
    char* content = NULL;
    
    output = CharBuffer_Create("From: <");
    formatSuccess = (e != NULL && output != NULL);
    
    //get pointers to the email fields
    sender = formatSuccess ? Email_GetSender(e) : NULL;
    receiver  = formatSuccess ? Email_GetReceiver(e) : NULL;
    subject = formatSuccess ? Email_GetSubject(e) : NULL;
    content = formatSuccess ? Email_GetContent(e) : NULL;
    
    //add sender
    formatSuccess = formatSuccess ? CharBuffer_Append(output, sender) : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, ">\r\n") : 0;
    
    //add receiver
    formatSuccess = formatSuccess ? CharBuffer_Append(output, "To: <") : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, receiver) : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, ">\r\n") : 0;
    
    //add subject
    formatSuccess = formatSuccess ? CharBuffer_Append(output, "Subject: ") : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, subject) : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, "\r\n") : 0;
    
    //add delimiter
    formatSuccess = formatSuccess ? CharBuffer_Append(output, "\r\n") : 0;
    
    //add content
    formatSuccess = formatSuccess ? CharBuffer_Append(output, content) : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, "\r\n") : 0;
    
    if (!formatSuccess) {
        fprintf(stderr, "Error formating email\n");
        CharBuffer_Delete(output);
        output = NULL;
    }
    else {
        //TODO: Andrei: Find logging mechanism
        //fprintf(stdout, "\nFormatted email:\n%s\n", CharBuffer_Buffer(output, "GET")); 
    }
    
    return output;
}

static size_t curlCallback(void* outputAddr,
    size_t elemSize, size_t elemCount,
    void* userData)
{
    int sendSuccess = 0;
    SmtpConnection* conn = NULL;
    const int outputSize = elemSize * elemCount;
    int bytesCopied = 0;
    
    sendSuccess = (outputAddr != NULL && userData != NULL);
    conn = sendSuccess ? (SmtpConnection*)userData : NULL;
    
    //the callback will be called twice. The first time, it will copy
    //the email buffer in the output address. The second time, it will
    //return 0 to signal to CURL that the transfer has been finished.
    if (sendSuccess && conn->currentEmail && !conn->emailSent) {
    
        CharBuffer* emailBuffer = formatEmail(conn->currentEmail);
        const int emailSize = CharBuffer_Size(emailBuffer);
        
        sendSuccess = (emailBuffer != NULL && emailSize <= outputSize);
        if (sendSuccess) {
            conn->emailSent = 1;
            bytesCopied = emailSize;
            memcpy(outputAddr, CharBuffer_Get(emailBuffer), bytesCopied);
            CharBuffer_Delete(emailBuffer);
        }    
    }
    
    return bytesCopied;
}

static CURL* createHandle(const char* addr,
    int port,
    int isSsl,
    const char* user,
    const char* pass)
{
    static int firstHandle = 1;
    CURL* output = NULL;
    int createSuccess = 0;
    char portString[6] = {0};
    CURLcode errorCode = CURLE_OK;
    
    if (firstHandle) {
        curl_global_init(CURL_GLOBAL_ALL);
        firstHandle = 0;
    }

    output = curl_easy_init();
    createSuccess = (output != NULL);

    if (createSuccess) {
        sprintf(portString, "%d", port);
        CharBuffer* addrAndPort = formatField(addr, ":", portString);
        createSuccess = (addrAndPort != NULL);

        errorCode = createSuccess ? 
            curl_easy_setopt(output, CURLOPT_URL, CharBuffer_Get(addrAndPort))
            : CURLE_FAILED_INIT;
        createSuccess = (errorCode == CURLE_OK);
        
        if (isSsl) {
            errorCode = createSuccess ?
                curl_easy_setopt(output, CURLOPT_USERNAME, user)
                : CURLE_FAILED_INIT;
            createSuccess = (errorCode == CURLE_OK);

            errorCode = createSuccess ?
                curl_easy_setopt(output, CURLOPT_PASSWORD, pass)
                : CURLE_FAILED_INIT;
            createSuccess = (errorCode == CURLE_OK);
        }

        CharBuffer_Delete(addrAndPort);
    }

    if (!createSuccess) {
        fprintf(stderr, "Error creating libcurl handle %s", curl_easy_strerror(errorCode));
        curl_easy_cleanup(output);
        output = NULL;
    }
    
    return output;
}

SmtpConnection* SmtpConnection_Create(int option, ...)
{
    int creationSuccess = 0;

    va_list parameters;
    char* addr = NULL;
    int port = 0;
    char* user = NULL;
    char* pass = NULL;

    SmtpConnection* output = NULL;
    CURL* handle = NULL;
    CharBuffer* addrBuffer = NULL;
    int isSsl = 0;
    CharBuffer* userBuffer = NULL;
    CharBuffer* passBuffer = NULL;
    Email* email = NULL;
    
    creationSuccess = (option == SMTP_SSL || option == SMTP_NO_SSL);
    if (creationSuccess) {
        //extract parameters after option
        va_start(parameters, option);
    }
    else {
        //fprintf(stderr, "SmtpConnection_Create invalid option\n");
    }

    if (creationSuccess && option == SMTP_NO_SSL) {
        addr = va_arg(parameters, char*);
        port = va_arg(parameters, int);

        creationSuccess = (addr != NULL && port > 0);
        if (!creationSuccess) {
            //fprintf(stderr, "SmtpConnection_Create invalid params for option %d\n", option);
        }
    }
    else if (creationSuccess && option == SMTP_SSL) {
        addr = va_arg(parameters, char*);
        port = va_arg(parameters, int);
        user = va_arg(parameters, char*);
        pass = va_arg(parameters, char*);

        creationSuccess = (addr != NULL && port > 0 && user != NULL && pass != NULL);
        if (!creationSuccess) {
            //fprintf(stderr, "SmtpConnection_Create invalid params for option %d\n", option);
        }
    }

    addrBuffer = creationSuccess ? CharBuffer_Create(addr) : NULL;
    creationSuccess = (addrBuffer != NULL);
    if (!creationSuccess) {
        fprintf(stderr, "SmtpConnection_Create cannot allocate memory for address\n");
    }

    isSsl = (option == SMTP_SSL);
    if (creationSuccess && isSsl) { 
        userBuffer = (creationSuccess) ? CharBuffer_Create(user) : NULL;
        creationSuccess = (userBuffer != NULL);
        if (!creationSuccess) {
            //fprintf(stderr, "SmtpConnection_Create cannot allocate memory for user buffer\n");
        }

        passBuffer = (creationSuccess) ? CharBuffer_Create(pass) : NULL;
        creationSuccess = (passBuffer != NULL);
        if (!creationSuccess) {
            //fprintf(stderr, "SmtpConnection_Create cannot allocate memory for password buffer\n");
        }
    }
    
    
    handle = creationSuccess ? createHandle(addr, port, isSsl, user, pass) : NULL;
    creationSuccess = (handle != NULL);
    if (!creationSuccess) {
        //fprintf(stderr, "SmtpConnection_Create error using libcurl\n");
    }

    output = creationSuccess ? malloc(sizeof(SmtpConnection)) : NULL;
    creationSuccess = (output != NULL);
    if (!creationSuccess) {
        //fprintf(stderr, "SmtpConnection_Create cannot allocate memory for SmtpConnection structure\n");
    }

    if (creationSuccess) {
        output->handle = handle;
        output->addr = addrBuffer;
        output->port = port;
        output->isSsl = isSsl;
        output->user = userBuffer;
        output->pass = passBuffer;
        output->currentEmail = email;
        output->emailSent = 0;
    }
    else {
        curl_easy_cleanup(handle);
        CharBuffer_Delete(addrBuffer);
        CharBuffer_Delete(userBuffer);
        CharBuffer_Delete(passBuffer);
    }

    va_end(parameters);

    return output;
}

void SmtpConnection_Delete(SmtpConnection* c)
{
    CURL* handle;
    CharBuffer* addr;
    int port;
    int isSsl;
    CharBuffer* user;
    CharBuffer* pass;

    if (c) {
        curl_easy_cleanup(c->handle);
        CharBuffer_Delete(c->addr);
        CharBuffer_Delete(c->user);
        CharBuffer_Delete(c->pass);
        Email_Delete(c->currentEmail); 
        free(c);
    }
}

int SmtpConnection_SendEmail(SmtpConnection* c, Email* e)
{
    int sendSuccess = 0;
    CURLcode curlCode = CURLE_FAILED_INIT;
    
    sendSuccess = (c!= NULL && e != NULL);
    
    if (sendSuccess) {
        c->currentEmail = e;
        
        //set all the options for the request
        struct curl_slist* receivers = makeReceiverList(Email_GetReceiver(e));
        curl_easy_setopt(c->handle, CURLOPT_MAIL_FROM, Email_GetSender(e));
        curl_easy_setopt(c->handle, CURLOPT_MAIL_RCPT, receivers);
        curl_easy_setopt(c->handle, CURLOPT_READFUNCTION, curlCallback);
        curl_easy_setopt(c->handle, CURLOPT_READDATA, c);
        /*
        int beginLine = 0;
        curl_easy_setopt(c->handle, CURLOPT_READDATA, c);
        curl_easy_setopt(c->handle, CURLOPT_READFUNCTION, testCallback);
        */
        curl_easy_setopt(c->handle, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(c->handle, CURLOPT_VERBOSE, 1L);
       
        //perform the request
        curlCode = curl_easy_perform(c->handle);
        sendSuccess = (curlCode == CURLE_OK);
        if (!sendSuccess) {
            fprintf(stderr, "CURL operation failed %s\n",
                    curl_easy_strerror(curlCode));
        }
        
        //cleanup
        c->currentEmail = NULL;
        c->emailSent = 0;
        Email_Delete(e);
        curl_slist_free_all(receivers);
    }
    
    return sendSuccess;
}
