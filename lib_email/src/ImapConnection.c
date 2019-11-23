#include "ImapConnection.h"
#include "lib_util/src/CharBuffer.h"

#include <curl/curl.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

const int IMAP_SSL = 0;
const int IMAP_NO_SSL = 1;
const int IMAP_SSL_DEFAULT_PORT = 993;
const int IMAP_NO_SSL_DEFAULT_PORT = 143;

struct _ImapConnection
{
    CURL* handle;
    CharBuffer* ip;
    int port;
    int isSsl;
    int fetchRequest;
    CharBuffer* user;
    CharBuffer* pass;
    CharBuffer* commandResult;
};

static CharBuffer* formatField(const char* prefix, const char* field, const char* suffix)
{
    int formatSuccess = 0;
    CharBuffer* output = NULL;
    
    output = CharBuffer_Create(prefix);
    formatSuccess = (output != NULL);
    
    formatSuccess = formatSuccess ? CharBuffer_Append(output, field) : 0;
    formatSuccess = formatSuccess ? CharBuffer_Append(output, suffix) : 0;
    
    if (!formatSuccess) {
        CharBuffer_Delete(output);
        output = NULL;
    }
    
    return output;
}

static CURL* createHandle(int isSsl, CharBuffer* ip, int port, CharBuffer* user, CharBuffer* pass)
{
    static int firstHandle = 0;
    
    CURL* output = NULL;
    int createSuccess = 0;
    CURLcode curlCode = CURLE_FAILED_INIT;
    CharBuffer* ipPort = NULL; //string with the ip and port in URL format ip:port
    char stringPort[6] = {0};
    
    if (firstHandle) {
        curl_global_init(CURL_GLOBAL_ALL);
        firstHandle = 1;
    }
    
    output = curl_easy_init();
    createSuccess = (output != NULL);
    
    //for default ports it is not necessary to add it to the URL
    if ((isSsl && port == IMAP_SSL_DEFAULT_PORT) || (!isSsl && port == IMAP_NO_SSL_DEFAULT_PORT)) {
        curlCode = createSuccess ? curl_easy_setopt(output, CURLOPT_URL, CharBuffer_Buffer(ip, "GET")) : CURLE_FAILED_INIT;
        createSuccess = (curlCode == CURLE_OK);
    }
    else {
        sprintf(stringPort, "%d", port);
        ipPort = createSuccess ? formatField(CharBuffer_Buffer(ip, "GET"), ":", stringPort) : NULL;
        curlCode = createSuccess ? curl_easy_setopt(output, CURLOPT_URL, CharBuffer_Buffer(ipPort, "GET")) : CURLE_FAILED_INIT;
        createSuccess = (curlCode == CURLE_OK);
    }
    
    if (isSsl) {
        curlCode = createSuccess ? curl_easy_setopt(output, CURLOPT_USERNAME, CharBuffer_Buffer(user, "GET")) : CURLE_FAILED_INIT;
        createSuccess = (curlCode == CURLE_OK);
        
        curlCode = createSuccess ? curl_easy_setopt(output, CURLOPT_PASSWORD, CharBuffer_Buffer(pass, "GET")) : CURLE_FAILED_INIT;
        createSuccess = (curlCode == CURLE_OK);
    }
    
    if (!createSuccess) {
        curl_easy_cleanup(output);
        output = NULL;
    }
    CharBuffer_Delete(ipPort);
    
    return output;
}

static int validateIp(CharBuffer* ip)
{
    char* dotPosition = strstr(CharBuffer_Buffer(ip, "GET"), ".");
    char* slashPosition = strstr(CharBuffer_Buffer(ip, "GET"), "//");
    
    return ((dotPosition != NULL) || (slashPosition != NULL));
}

static char* parseFetchResponse(char* response, size_t responseLen)
{
    char* output = NULL;
    int parseSuccess = 0;
    //the content of the email is not part of the responseLen
    //the length of the response is between {} accolades.
    long int fetchLen = 0;
    char* begining = NULL;
    char* ending = NULL;
        
    begining = memchr(response, '{', responseLen);
    parseSuccess = (begining != NULL);
        
    //convert number after '{'
    fetchLen = parseSuccess ? strtol(begining + 1, &ending, 10) : 0;
    parseSuccess = (fetchLen > 0 && fetchLen < LONG_MAX && ending != NULL);
    
    output = parseSuccess ? malloc(fetchLen + 1) : NULL; //space for \0
    parseSuccess = (output != NULL);
        
    if (parseSuccess) {
        //the content of the resonse if after '}'
        memcpy(output, ending + 1, fetchLen);
        output[fetchLen] = '\0';
    }
    
    return output;
}

static size_t curlCallback(char* receivedData, size_t elemSize, size_t elemCount, void* userData)
{
    int readSuccess = 0;
    size_t bytesProcessed = 0; // if different than elemCount*elemSize, signals error to libCURL
    
    readSuccess = (receivedData != NULL && elemSize != 0 && elemCount != 0);
    
    if (readSuccess) {
        ImapConnection* connection = (ImapConnection*)userData;
        char* data = NULL;
        CharBuffer* dataBuffer = NULL;
        char* fetchResponse = NULL;
        
        //put the received data in another buffer.
        data = malloc((elemCount * elemSize) +1); //1 for NULL terminator
        readSuccess = (data != NULL);
        if (readSuccess) {
            memcpy(data, receivedData, elemSize * elemCount);
            data[elemSize * elemCount] = '\0';
        }
        
        if (readSuccess && connection->commandResult == NULL) {
            //the ownership of the data has been passed to the CharBuffer.
            dataBuffer = readSuccess ? CharBuffer_Take(&data) : NULL;
            readSuccess = (dataBuffer != NULL);
            connection->commandResult = readSuccess ? dataBuffer : NULL;
        }
        else if (readSuccess && connection->commandResult != NULL) {
            CharBuffer_Append(connection->commandResult, data);
            free(data);
        }
        
        //the response to FETCH commands is stored after elemCount*elemSize
        //parse the response and get the content
        if (readSuccess && connection->fetchRequest) {
            fetchResponse = parseFetchResponse(receivedData, elemSize * elemCount);
            if (fetchResponse) {
                CharBuffer_Append(connection->commandResult, fetchResponse);
                free(fetchResponse);
            }
        }
        
        if (!readSuccess) {
            if (dataBuffer) {
                CharBuffer_Delete(dataBuffer);
            } else if (data) {
                free(data);
            }
        }
    }
    
    bytesProcessed = readSuccess ? (elemCount * elemSize) : 0;
    return bytesProcessed;
}

ImapConnection* ImapConnection_Create(int option, ...)
{
    //pointers to expected input data
    char* ipPtr = NULL;
    char* userPtr = NULL;
    char* passPtr = NULL;
    
    //output data and its internal representation
    ImapConnection* output = NULL;
    CURL* handle = NULL;
    CharBuffer* ip;
    int port = -1;
    int isSsl = 0;
    CharBuffer* user = NULL;
    CharBuffer* pass = NULL;
    CharBuffer* commandResult = NULL;
    
    int createSuccess = 0;
    va_list args;
    
    va_start(args, option);
    
    createSuccess = (option == IMAP_NO_SSL || option == IMAP_SSL);
    isSsl = (option == IMAP_SSL);
    if (!createSuccess) {
        //TODO: Andrei: find a way to log errors.
        fprintf(stderr, "Wrong option for ImapConnection_Create\n");
    }
    
    ipPtr = createSuccess ? va_arg(args, char*) : NULL;
    ip = createSuccess ? CharBuffer_Create(ipPtr) : NULL;
    createSuccess = (ip != NULL && validateIp(ip));
    
    port = createSuccess ? va_arg(args, int) : -1;
    createSuccess = (port != -1);
    
    if (isSsl) {
    
        userPtr = createSuccess ? va_arg(args, char*) : NULL;
        user = createSuccess ? CharBuffer_Create(userPtr) : NULL;
        createSuccess = (user != NULL);
        
        passPtr = createSuccess ? va_arg(args, char*) : NULL;
        pass = createSuccess ? CharBuffer_Create(passPtr) : NULL;
        createSuccess = (pass != NULL);
    }
    
    handle = createSuccess ? createHandle(isSsl, ip, port, user, pass) : NULL;
    createSuccess = (handle != NULL);
    
    output = createSuccess ? malloc(sizeof(ImapConnection)) : NULL;
    createSuccess = (output != NULL);
    if (createSuccess) {
        output->handle = handle;
        output->ip = ip;
        output->port = port;
        output->isSsl = isSsl;
        output->user = user;
        output->pass = pass;
        output->commandResult = commandResult;
        output->fetchRequest = 0;
    }
    else {
        curl_easy_cleanup(handle);
        CharBuffer_Delete(ip);
        CharBuffer_Delete(user);
        CharBuffer_Delete(pass);
        output = NULL;
    }
    
    return output;
}

void ImapConnection_Delete(ImapConnection* c)
{
    if (c) {
        curl_easy_cleanup(c->handle);
        CharBuffer_Delete(c->ip);
        CharBuffer_Delete(c->user);
        CharBuffer_Delete(c->pass);
        CharBuffer_Delete(c->commandResult);
        
        free(c);
        c = NULL;
    }
}

const char* ImapConnection_ExecuteCommand(ImapConnection* connection, const char* command)
{
    char* output = NULL;
    
    int commandSuccess = 0;
    CURLcode curlCode = CURLE_FAILED_INIT;
    const char* fetchPosition = NULL;
    
    commandSuccess = (connection != NULL && command != NULL);
    
    curlCode = commandSuccess ? curl_easy_setopt(connection->handle, CURLOPT_WRITEDATA, connection) : CURLE_FAILED_INIT;
    commandSuccess = (curlCode == CURLE_OK);
    
    curlCode = commandSuccess ? curl_easy_setopt(connection->handle, CURLOPT_WRITEFUNCTION, curlCallback) : CURLE_FAILED_INIT;
    commandSuccess = (curlCode == CURLE_OK);
    
    curlCode = commandSuccess ? curl_easy_setopt(connection->handle, CURLOPT_CUSTOMREQUEST, command) : CURLE_FAILED_INIT;
    commandSuccess = (curlCode == CURLE_OK);
    
    fetchPosition = commandSuccess ? strstr(command, "FETCH") : NULL;
    if (commandSuccess) {
        connection->fetchRequest = (fetchPosition != NULL);
    }
    
    curlCode = commandSuccess ? curl_easy_perform(connection->handle) : CURLE_FAILED_INIT;
    commandSuccess = (curlCode == CURLE_OK);
    
    output = commandSuccess ? CharBuffer_Buffer(connection->commandResult, "COPY") : NULL;
    
    if (commandSuccess) {
        //clean up the buffer for the next command.
        CharBuffer_Delete(connection->commandResult);
        connection->commandResult = NULL;
    }
    return output;
}
