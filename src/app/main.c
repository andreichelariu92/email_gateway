#include <stdio.h>
#include <string.h>

#include <curl/curl.h>

struct curl_slist* makeList(const char* name)
{
    struct curl_slist* output = NULL;
    output = curl_slist_append(output, name);

    return output;
}

FILE* openFile(const char* fileName)
{
    FILE* output = NULL;

    output = fopen(fileName, "r");

    if (!output) {
       fprintf(stderr, "Error opening file %s\n", fileName); 
    }

    return output;
}

static const char* mailLines[] = {
    "From: " "<andreichelariu92@yahoo.com>" "\r\n",
    "To: " "<andreichelariu92@gmail.com>" "\r\n",
    "Subject: Pula\r\n",
    "\r\n",
    "Sa vedem sa vedem ce-o sa iasa.\r\n",
    NULL
};

size_t callback(void* addr, 
                size_t elemSize, 
                size_t elemCount,
                void* userData)
{
    int fail = 0;
    size_t lineSize = 0;
    int* lineIdx = 0;

    fail = (elemSize == 0) || (elemCount == 0) || (userData == 0);
    if (!fail) {
        lineIdx = (int*) userData;
        if (mailLines[*lineIdx]) {
            lineSize = strlen(mailLines[*lineIdx]);
            memcpy(addr, mailLines[*lineIdx], lineSize);
            (*lineIdx)++;
        }
    }
    
    return lineSize;
}

size_t writeCallback(char* receivedData, size_t elemSize, size_t elemCount, void* userData)
{
    printf("C'est moi, motherfucker\n");
    printf("%s\n", receivedData);
    
    return (elemCount * elemSize);
}

int main()
{
    /*
    CURL* handle = NULL;
    CURLcode result = CURLE_OK;
    const char* from = "<andreichelariu92@yahoo.com>";
    const char* to = "<andreichelariu92@gmail.com>";
    struct curl_slist* recipents = makeList(to);
    FILE* mailContent = //openFile("./mail.txt");
    handle = curl_easy_init();
    int lineIdx = 0;

    if (handle && mailContent) {
        curl_easy_setopt(handle,
                CURLOPT_URL,
                "smtps://smtp.gmail.com");
        
        curl_easy_setopt(handle, 
                CURLOPT_USERNAME, 
                "andreichelariu92");
        curl_easy_setopt(handle,
                CURLOPT_PASSWORD,
                "PRIVATE");
        
        curl_easy_setopt(handle, CURLOPT_MAIL_FROM, from);
        curl_easy_setopt(handle, CURLOPT_MAIL_RCPT, recipents);
        
        curl_easy_setopt(handle, CURLOPT_READFUNCTION, callback);
        curl_easy_setopt(handle, CURLOPT_READDATA, &lineIdx);
        curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
        
        curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);

        result = curl_easy_perform(handle);

        if (result != CURLE_OK) {
            fprintf(stderr, "operation failed %s\n",
                    curl_easy_strerror(result));
        }

        curl_slist_free_all(recipents);
        curl_easy_cleanup(handle);

    }
    return 0;
    */
    
    CURL* handle;
    CURLcode success = CURLE_OK;
    
    handle = curl_easy_init();
    success = handle ? CURLE_OK : CURLE_FAILED_INIT;
    
    if (success == CURLE_OK) {
        curl_easy_setopt(handle, CURLOPT_USERNAME, "testemailgateway");
        curl_easy_setopt(handle, CURLOPT_PASSWORD, "PRIVATE");
        
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, NULL);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
        
        curl_easy_setopt(handle, CURLOPT_URL, "imaps://imap.mail.yahoo.com/INBOX");
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "FETCH 3 (FLAGS BODY[TEXT])");
        //curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
        
        success = curl_easy_perform(handle);
        
        
        curl_easy_cleanup(handle);
    }
    
    return 0;
}
