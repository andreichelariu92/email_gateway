#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MinUnit.h"

#include "lib_util/src/MemoryRegion.h"
#include "lib_util/src/CharBuffer.h"

#include "lib_email/src/Email.h"
#include "lib_email/src/SmtpConnection.h"
#include "lib_email/src/ImapConnection.h"

char* test_MemoryRegion_Allocate()
{
    MemoryRegion* m1 = MemoryRegion_Allocate(1, 1);
    mu_assert("Allocate valid MemoryRegion", (m1 != 0));
    MemoryRegion_Delete(m1);

    MemoryRegion* m2 = MemoryRegion_Allocate(1, -1);
    mu_assert("MemoryRegion_Allocate invalid element size", 
        (m2 == 0));
    MemoryRegion_Delete(m2);

    MemoryRegion* m3 = MemoryRegion_Allocate(1, 0);
    mu_assert("MemoryRegion_Allocate element size zero", (m3 == 0));
    MemoryRegion_Delete(m3);

    return 0;
}

char* test_MemoryRegion_Take()
{
    int* buff = malloc(10*sizeof(int));
    MemoryRegion* m1 = 
        MemoryRegion_Take((void**)(&buff), 10, sizeof(int));
        
    mu_assert("MemoryRegion_Take input buffer becomes NULL", 
        (buff == 0));
    mu_assert("MemoryRegion_Take return value is not NULL", 
        (m1 != 0));
    MemoryRegion_Delete(m1);

    return 0;
}

char* test_MemoryRegion_Buffer()
{
    MemoryRegion* m1 = MemoryRegion_Allocate(1, 1);
    char* buff = MemoryRegion_Buffer(m1);
    mu_assert("MemoryRegion_Buffer failed for valid input", (buff != 0));
    MemoryRegion_Delete(m1);
    
    MemoryRegion* m2 = MemoryRegion_Allocate(-11, 1);
    char* buff2 = MemoryRegion_Buffer(m2);
    mu_assert("MemoryRegion_Buffer for invalid input failed", (buff2 == 0));
    MemoryRegion_Delete(m2);

    return 0;
}

char* test_MemoryRegion_ElementCount()
{
    MemoryRegion* m = MemoryRegion_Allocate(100, 1);
    const size_t nrElem1 = MemoryRegion_ElementCount(m);
    mu_assert("MemoryRegion_ElementCount error for valid input", (nrElem1 == 100));
    MemoryRegion_Delete(m);

    MemoryRegion* m2 = MemoryRegion_Allocate(-1, -200);
    const size_t nrElem2 = MemoryRegion_ElementCount(m2);
    mu_assert("MemoryRegion_ElementCount error for invalid input", (nrElem2 == 0));
    MemoryRegion_Delete(m2);

    return 0;
}

char* test_All1()
{
    const char command[] = "SELECT * FROM boys;";
    MemoryRegion* chars = 
        MemoryRegion_Allocate(strlen(command) + 1, 1);
    mu_assert("MemoryRegion_Allocate failed", (chars != 0));
    
    strcpy(MemoryRegion_Buffer(chars), command);
    
    int charIdx = 0;
    const int charsLen = MemoryRegion_ElementCount(chars);
    for (; charIdx < charsLen; ++charIdx) {
        void* charAddr = MemoryRegion_At(chars, charIdx);
        mu_assert("Invalid content of MemoryBuffer", (command[charIdx] == *((char*)(charAddr))));
    }

    MemoryRegion_Delete(chars);

    return 0;
}

char* test_All2()
{
    MemoryRegion* numbers = MemoryRegion_Allocate(3, sizeof(int));
    mu_assert("MemoryRegion_Allocate failed", (numbers != 0));

    int numbersArray[] = {1, 2, 3};
    int numberIdx = 0;
    for (; numberIdx < 3; ++numberIdx) {
        void* numberAddr = 
            MemoryRegion_At(numbers, numberIdx);
        *((int*)(numberAddr)) = numbersArray[numberIdx];
    }
    
    int* memoryRegionBuffer = MemoryRegion_Buffer(numbers);
    for (numberIdx = 0; numberIdx < 3; ++numberIdx) {
        mu_assert("Invalid content of MemoryRegion", (memoryRegionBuffer[numberIdx] == numbersArray[numberIdx]));
    }

    MemoryRegion_Delete(numbers);

    return 0;
}

char* test_CharBuffer_Copy()
{
    char command[] = "SELECT name FROM boys;";
    CharBuffer* c1 = CharBuffer_Copy(command);
    mu_assert("CharBuffer_Copy failed for valid input", (c1 != 0));
    CharBuffer_Delete(c1);

    CharBuffer* c2 = CharBuffer_Copy(0);
    mu_assert("CharBuffer_Copy failed for valid input", (c2 == 0));
    CharBuffer_Delete(c2);

    return 0;
}

char* test_CharBuffer_Take()
{
    char command[] = 
        "INSERT INTO boys VALUES(1, 'Andrei', 'Chelariu');";
    char* commandOnHeap = malloc(strlen(command) + 1);
    strcpy(commandOnHeap, command);

    CharBuffer* c = CharBuffer_Take(&commandOnHeap);
    mu_assert("CharBuffer_Take did not clear the input parameter", (commandOnHeap == 0));
    mu_assert("CharBuffer_Take failed", (c != 0));
    CharBuffer_Delete(c);

    return 0;
}

char* test_CharBuffer_Buffer()
{
    char* c1 = CharBuffer_Buffer(0, "get");
    mu_assert("CharBuffer_Buffer failed", (c1 == 0));

    CharBuffer* cb = CharBuffer_Copy("Ana are raie");
    mu_assert("Getter function for CharBuffer failed", (CharBuffer_Buffer(cb, "get") != 0));
    CharBuffer_Delete(cb);
    
    CharBuffer* cb2 = CharBuffer_Copy("Just a test");
    
    char* copyBuff = CharBuffer_Buffer(cb2, "copy");
    mu_assert("Copy option for CharBuffer_Buffer failed", (copyBuff != 0));
    free(copyBuff);

    const char* invalidBuff = CharBuffer_Buffer(cb2, "invalid");
    mu_assert("Invalid option for CharBuffer_Buffer failed", (invalidBuff == 0));

    CharBuffer_Delete(cb2);

    return 0;
}

char* test_CharBuffer_All()
{
    CharBuffer* c1 = CharBuffer_Copy("Andrei Chelariu");
    CharBuffer* c2 = CharBuffer_Copy("Stefan Mechenici");
    CharBuffer* c3 = CharBuffer_Copy("Sebi Andone");

    CharBuffer* total = CharBuffer_Copy("");
    CharBuffer_Append(total, CharBuffer_Buffer(c1, "get"));
    CharBuffer_Append(total, ", ");
    CharBuffer_Append(total, CharBuffer_Buffer(c2, "GET"));
    CharBuffer_Append(total, ", ");
    CharBuffer_Append(total, CharBuffer_Buffer(c3, "get"));

    const int totalSize = strlen(CharBuffer_Buffer(total, "get"));
    const int testValue = 
        strlen("Andrei Chelariu, Stefan Mechenici, Sebi Andone");
    mu_assert("test_CharBuffer_All failed", (totalSize == testValue));

    CharBuffer_Delete(c1);
    CharBuffer_Delete(c2);
    CharBuffer_Delete(c3);
    CharBuffer_Delete(total);

    return 0;
}

char* test_Email_Create()
{
    {
        // test case 1:
        // Create an email structure with invalid input (NULL)
        // The function should return error (NULL)
        Email* e = Email_Create(NULL, 
                NULL,
                NULL,
                NULL);
        mu_assert("Email_Create with NULL input", (e == 0));
    }

    {
         // test case 2:
         // Create an email structure with invalid sender
         // (the sender must have the following form:
         // username@server.com)
         // The function should return null.
        const char invalidSender[] = "invalid";
        const char validReceiver[] = "receiver@test.com";
        const char validSubject[] = "Subject";
        const char validContent[] = "Content";

        Email* e = Email_Create(invalidSender,
                validReceiver,
                validSubject,
                validContent);

        mu_assert("Email_Create with invalid sender", (e == 0));
    }

    {
         // test case 3:
         // Create an email structure with valid input.
         // The return value should be different from zero.
        const char validSender[] = "sender@test.com";
        const char validReceiver[] = "receiver@test.com";
        const char validSubject[] = "Subject";
        const char validContent[] = "Content";
        
        Email* e = Email_Create(validSender,
                validReceiver,
                validSubject,
                validContent);

        mu_assert("Email_Create with valid input", (e != 0));

        Email_Delete(e);
    }

    return 0;
}

char* test_Email_Getters()
{
    {
         // test case 1:
         // Create an email structure with invalid input.
         // All the getters should return null, for get,
         // copy and invalid options.
        const char* options[] = {
            "get",
            "copy",
            "invalid option"
        };
        const int optionCount = 3;

        Email* e = Email_Create(NULL,
                NULL,
                NULL,
                NULL);
        
        int optionIdx = 0;
        for (; optionIdx < optionCount; ++optionIdx) {
            mu_assert("Email_Sender for invalid input", (Email_Sender(e, options[optionIdx]) == NULL));
            mu_assert("Email_Receiver for invalid input", (Email_Receiver(e, options[optionIdx]) == NULL));
            mu_assert("Email_Subject for invalid input", (Email_Subject(e, options[optionIdx]) == NULL));
            mu_assert("Email_Content for invalid input", (Email_Content(e, options[optionIdx]) == NULL));
        }
    }

    {
         // test case 2:
         // Create an email structure with invalid input.
         // The getters should return the exptected input
         // with "get" and "copy" options
        
        const char sender[] = "stefan@bossu.com";
        const char receiver[] = "andrei@bossu.com";
        const char subject[] = "Test";
        const char content[] = "Salut Stefan";

        Email* e = Email_Create(sender,
            receiver,
            subject,
            content);

        mu_assert("Email_Create for valid input", (e != NULL));

        const char* gSender = Email_Sender(e, "get");
        const char* gReceiver = Email_Receiver(e, "get");
        char* cSubject = Email_Subject(e, "copy");
        char* cContent = Email_Content(e, "copy");
        char* iSender = Email_Sender(e, "invalid_opt");

        mu_assert("Email_Sender for valid input", (strcmp(sender, gSender) == 0));
        mu_assert("Email_Receiver for valid input", (strcmp(receiver, gReceiver) == 0));
        mu_assert("Email_Subject for valid input", (strcmp(subject, cSubject) == 0));
        mu_assert("Email_Content for valid input", (strcmp(content, cContent) == 0));
        mu_assert("Email_Content for invalid option", (iSender == NULL));

        free(cSubject);
        free(cContent);

        Email_Delete(e);
    }

    return 0;
}

char* test_SmtpConnection_Create()
{
    {
        //test case 1:
        //create a connection with an invalid user
        //the return value should be NULL
        SmtpConnection* c = 
            SmtpConnection_Create(SMTP_NO_SSL, 0, 433);
        mu_assert("Create SmtpConnection with invalid user", (c == NULL));
    }

    {
        //test case 2:
        //create a connection with an invalid port
        //the return value should be NULL
        SmtpConnection* c =
            SmtpConnection_Create(SMTP_NO_SSL, "192.168.1.102", -22);
        mu_assert("Create SmtpConnection with invalid port", (c == NULL));
    }

    {
        //test case 3:
        //create a SSL connection with invalid input data
        const char* invalidAddr = NULL;
        const int invalidPort = -1;
        const char* invalidUser = 0;
        const char* invalidPass = 0;

        SmtpConnection* c =
            SmtpConnection_Create(SMTP_SSL, NULL, invalidUser, invalidPass);
        mu_assert("Create SSL SMTP connection with invalid data", (c == NULL));
    }

    {
        //test case 4:
        //create two connections with valid parameters. The return values
        //should be different from NULL.
        const char* addr = "192.168.1.102";
        const int port = 443;
        const char* user = "theUser";
        const char* pass = "lePass";

        SmtpConnection* c1 = SmtpConnection_Create(SMTP_NO_SSL, addr, port);
        SmtpConnection* c2 = SmtpConnection_Create(SMTP_SSL, addr, port, user, pass);

        mu_assert("Create connections with valid input params", (c1 != NULL && c2 != NULL));

        SmtpConnection_Delete(c1);
        SmtpConnection_Delete(c2);
    }
    return 0;
}

char* test_SmtpConnection_SendEmail2()
{
    SmtpConnection* invalidConnection1 = NULL;
    SmtpConnection* localConnection = SmtpConnection_Create(SMTP_SSL, "127.0.0.1", 465, "andrei", "PRIVATE");
    const char* invalidSender = NULL;
    const char* invalidReceiver = "receiver";

    SmtpConnection* validConnection = SmtpConnection_Create(SMTP_SSL, "smtps://smtp.gmail.com", 465, "andreichelariu92", "PRIVATE");
    const char validSender[] = "testemailgateway@yahoo.com";
    const char validReceiver[] = "andreichelariu92@gmail.com";
    const char validSubject[] = "Test Email gateway";
    const char validContent[] = "It works!";
    
    {
        //test case 1:
        //send an email to a NULL connection
        //the return value should be false (0)

        int success = SmtpConnection_SendEmail(invalidConnection1, 
                validSender, 
                validReceiver, 
                validSubject, 
                validContent);
        mu_assert("Send email to an invalid connection\n", (success == 0));
    }

    {
        //test case 2:
        //create a connection with a valid ip and port, but
        //with no SMTP server.
        //the return value should be false (0)
        int success = SmtpConnection_SendEmail(localConnection,
                validSender,
                validReceiver,
                validSubject,
                validContent);
        mu_assert("Send email to an address with no SMTP server\n", (success == 0));
    }

    {
        //test case 3:
        //create a valid connection and send an invalid email (invalid sender
        //and receiver)
        //the return value should be false (0)
        int success = SmtpConnection_SendEmail(validConnection,
                invalidSender,
                invalidReceiver,
                validSubject,
                validContent);
        mu_assert("Send email to a valid connection, but with invalid information\n", (success == 0));
    }
    /*
    {
        //test case 4:
        //create a valid connection and send a valid email
        //the return value should be true (1)
        int success = SmtpConnection_SendEmail(validConnection,
                validSender,
                validReceiver,
                validSubject,
                validContent);
        mu_assert("Send valid email to valid conneciton\n", success);
    }
    */

    SmtpConnection_Delete(localConnection);
    SmtpConnection_Delete(validConnection);
    
    return 0;
}

char* test_ImapConnection_Create()
{
    //test case 1:
    //create a connection with invalid option parameter.
    //the return value should be NULL.
    {
        const int invalidOption = -1;
        ImapConnection* c = ImapConnection_Create(invalidOption);
        mu_assert("Create IMAP connection with invalid option\n", (c == NULL));
        
    }
    
    //test case 2:
    //create a non ssl connection with invalid parameters.
    //the return value should be NULL.
    {
        const int VALID_IDX = 0;
        const int INVALID_IDX = 1;
        const int ports[] = {993, -1};
        const char* ips[] = {"127.0.0.1", NULL};
        
        ImapConnection* c1 = ImapConnection_Create(IMAP_NO_SSL, ips[VALID_IDX], ports[INVALID_IDX]);
        ImapConnection* c2 = ImapConnection_Create(IMAP_NO_SSL, ips[INVALID_IDX], ports[VALID_IDX]);
        mu_assert("Create non SSL IMAP connection with invalid params\n", (c1 == NULL && c2 == NULL));
    }
    
    //test case 3:
    //create ssl imap connection with invalid parameters.
    //the return value should be NULL.
    {
        const int VALID_IDX = 0;
        const int INVALID_IDX = 1;
        const int ports[] = {993, -1};
        const char* ips[] = {"127.0.0.1", "wrong"};
        const char* users[2] = {"user", NULL};
        const char* passwords[] = {"pass", NULL};
        
        const int PORT_MASK = 0;
        const int IP_MASK = 1;
        const int USER_MASK = 2;
        const int PASSWORD_MASK = 3;
        
        int optionIdx = 0;
        for (; optionIdx < 4; ++optionIdx) {
           ImapConnection* c = ImapConnection_Create(IMAP_SSL, 
                                                      ips[optionIdx == IP_MASK],
                                                      ports[optionIdx == PORT_MASK],
                                                      users[optionIdx == USER_MASK],
                                                      passwords[optionIdx == PASSWORD_MASK]
                                                     );
            //ImapConnection* c = ImapConnection_Create(IMAP_SSL, "127.0.0.1", 993, users[1], NULL);
            mu_assert("Create IMAP SSL connection with invalid params\n", (c == NULL));
            
        }
    }
    
    //test case 4:
    //create imap connections with valid parameters.
    //the return values should be different from NULL.
    {
        const char* validIp = "127.0.0.1";
        const int validPort = 993;
        const char* validUser = "user";
        const char* validPass = "password";
        
        ImapConnection* i1 = ImapConnection_Create(IMAP_NO_SSL, validIp, validPort);
        ImapConnection* i2 = ImapConnection_Create(IMAP_SSL, validIp, validPort, validUser, validPass);
        
        mu_assert("Create IMAP connections with valid params\n", (i1 != NULL && i2 != NULL));
        
        ImapConnection_Delete(i1);
        ImapConnection_Delete(i2);
    }
    
    return 0;
}

char* test_ImapConnectionExecuteCommand()
{
    //test case 1:
    //execute a command on an invalid connection (NULL).
    //the result should be NULL.
    {
        ImapConnection* invalidConnection = NULL;
        const char* validCommand = "SELECT INBOX";
        
        const char* result = ImapConnection_ExecuteCommand(invalidConnection, validCommand);
        mu_assert("Execute IMAP command on NULL connection\n", (result == NULL));
        
        ImapConnection_Delete(invalidConnection);
    }
    
    //test case 2:
    //execute an invalid command (NULL) on a valid connection.
    //the result should be NULL.
    {
        ImapConnection* validConnection = ImapConnection_Create(IMAP_NO_SSL, "127.0.0.1", 993);
        const char* invalidCommand = NULL;
        
        const char* result = ImapConnection_ExecuteCommand(validConnection, invalidCommand);
        mu_assert("Execute invalid IMAP command on valid connection\n", (result == NULL));
        
        ImapConnection_Delete(validConnection);
    }
    
    //test case 3:
    //execute a valid command on a connection that does not support IMAP protocol.
    //the result should be NULL.
    {
        ImapConnection* localConnection = ImapConnection_Create(IMAP_NO_SSL, "127.0.0.1", 993);
        const char* validCommand = "SELECT INBOX";
        
        const char* result = ImapConnection_ExecuteCommand(localConnection, validCommand);
        mu_assert("Execute IMAP command on a connection that does not handle IMAP\n", (result == NULL));
        
        ImapConnection_Delete(localConnection);
    }
    
    /*
    //TODO: Andrei: remove personal data before commit to github.
    //test case 4:
    //execute a valid command on an IMAP server.
    //the result should be NOT NULL.
    {
        ImapConnection* validConnection = ImapConnection_Create(IMAP_SSL, "imaps://imap.mail.yahoo.com/", IMAP_SSL_DEFAULT_PORT, "testemailgateway", "PRIVATE");
        const char* validCommand = "SELECT INBOX";
        
        const char* result = ImapConnection_ExecuteCommand(validConnection, validCommand);
        mu_assert("Execute IMAP command on a valid IMAP connection\n", (result != NULL));
        
        printf("%s\n", result);
        ImapConnection_Delete(validConnection);
    }
    */
    
    return 0;
}

int tests_run = 0;

char* runAllTests()
{
    mu_run_test(test_MemoryRegion_Allocate);
    mu_run_test(test_MemoryRegion_Take);
    mu_run_test(test_All1);
    mu_run_test(test_All2);
    mu_run_test(test_MemoryRegion_Buffer);
    mu_run_test(test_MemoryRegion_ElementCount);

    mu_run_test(test_CharBuffer_All);
    mu_run_test(test_CharBuffer_Buffer);
    mu_run_test(test_CharBuffer_Take);
    mu_run_test(test_CharBuffer_Copy);

    mu_run_test(test_Email_Getters);
    mu_run_test(test_Email_Create);

    mu_run_test(test_SmtpConnection_Create);
    mu_run_test(test_SmtpConnection_SendEmail2);
    
    mu_run_test(test_ImapConnection_Create);
    mu_run_test(test_ImapConnectionExecuteCommand);
    return 0;
}

int main()
{
    const char* error = runAllTests();
    printf("Number of executed tests: %d\n", tests_run);
    if (error) {
        printf("Tests failed\n %s\n", error);
    }
    else {
        printf("ALL TESTS PASSED :)\n");
    }

    return 0;
}
