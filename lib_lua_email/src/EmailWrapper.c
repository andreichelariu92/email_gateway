#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "lib_email/src/SmtpConnection.h"
#include "lib_email/src/ImapConnection.h"

/**
 * \brief
 * Wrapper for the function that creates a SmtpConnection.
 * Parameters from the interpreter:
 * 1) smtp flag (boolean)
 * 2) ip (string)
 * 3) port (number)
 * 4) user (string)
 * 5) pass (string)
 * 
 * In case of error, it returns NIL.
 */
static int makeSmtp(lua_State* L)
{
    int sslFlag = 0;
    const char* ip = NULL;
    int port = 0;
    int smtpSuccess = 0;
    SmtpConnection* connection = NULL;
    
    sslFlag = lua_toboolean(L, 1);
    
    ip = lua_tolstring(L, 2, NULL);
    smtpSuccess = (ip != NULL);
    
    port = smtpSuccess ? lua_tointegerx(L, 3, NULL) : 0;
    smtpSuccess = (port > 0);
    
    if (sslFlag && smtpSuccess) {
        const char* user = NULL;
        const char* pass = NULL;
        
        user = lua_tolstring(L, 4, NULL);
        smtpSuccess = (user != NULL);
        
        pass = smtpSuccess ? lua_tolstring(L, 5, NULL) : NULL;
        smtpSuccess = (pass != NULL);
        
        connection = smtpSuccess ? SmtpConnection_Create(SMTP_SSL, ip, port, user, pass) : NULL;
        smtpSuccess = (connection != NULL);
    }
    else if (smtpSuccess) {
        connection = SmtpConnection_Create(SMTP_NO_SSL, ip, port);
        smtpSuccess = (connection != NULL);
    }
    
    if (smtpSuccess) {
        // store the address of the smtp connection inside the interpreter.
        SmtpConnection** interpreterAddress = lua_newuserdata(L, sizeof(SmtpConnection*));
        smtpSuccess = (interpreterAddress != NULL);
        
        if (smtpSuccess) {
            *interpreterAddress = connection;
            luaL_getmetatable(L, "lua_email_smtp");
            lua_setmetatable(L, -2);
        }
    }
   
    if (smtpSuccess) {
        //TODO: Andrei: Find logging mechanism
        printf("Smtp connection created successfully: ip=%s, port=%d\n", ip, port);
        return 1;
    }
    else {
        //cleanup
        if (connection) {
            SmtpConnection_Delete(connection);
        }
        return luaL_error(L, "Error creating smtp connection");
    }
}

/**
 * \brief
 * Function that sends an email using a SMTP connection (see above)
 * Parameters from lua:
 * - connection
 * - sender
 * - receiver
 * - subject
 * - content
 */
static int sendEmail(lua_State* L)
{
    int sendSuccess = 0;
    SmtpConnection** connectionAddr = NULL;
    const char* sender = NULL;
    const char* receiver = NULL;
    const char* subject = NULL;
    const char* content = NULL;
    
    connectionAddr = luaL_checkudata(L, 1, "lua_email_smtp");
    sendSuccess = (connectionAddr != NULL);
    
    sender = sendSuccess ? lua_tolstring(L, 2, NULL) : NULL;
    sendSuccess = (sender != NULL);
    
    receiver = sendSuccess ? lua_tolstring(L, 3, NULL) : NULL;
    sendSuccess = (receiver != NULL);
    
    subject = sendSuccess ? lua_tolstring(L, 4, NULL) : NULL;
    sendSuccess = (subject != NULL);
    
    content = sendSuccess ? lua_tolstring(L, 5, NULL) : NULL;
    sendSuccess = (content != NULL);
    
    Email* e = Email_Create(sender, receiver, subject, content);
    if (sendSuccess && e) {
        sendSuccess = SmtpConnection_SendEmail(*connectionAddr, e);
        lua_pushboolean(L, sendSuccess);
        return 1;
    }
    else {
        //TODO: Andrei: find logging mechanism.
        fprintf(stderr, "Invalid parameters for send email\n");
        return luaL_error(L, "Invalid parameters for sendEmail method");
    }
}

/**
 * \brief
 * Function called by the garbage collector to delete SMTP connection.
 */
static int deleteSmtp(lua_State* L)
{
    SmtpConnection** connectionAddr = NULL;
    
    connectionAddr = lua_touserdata(L, 1);
    SmtpConnection_Delete(*connectionAddr);
    
    //TODO: Andrei: find logging mechanism.
    printf("SMTP garbage collector called\n");
    
    return 0;
}

/**
 * \brief
 * Function that creates an IMAP connection.
 * Parameters received from the interpreter:
 * type: true -> SSL false -> NO_SSL
 * ip
 * port
 * user
 * pass
 * Returns a new imap connection or raises error.
 */
static int makeImap(lua_State* L)
{
    int imapSuccess = 0;
    int isSsl = 0;
    const char* ip = NULL;
    int port = 0;
    ImapConnection* connection = NULL;
    
    isSsl = lua_toboolean(L, 1);
    
    ip = lua_tolstring(L, 2, NULL);
    imapSuccess = (ip != NULL);
    
    port = imapSuccess ? lua_tonumber(L, 3) : 0;    
    imapSuccess = (port != 0);
    
    if (isSsl && imapSuccess) {
        const char* user = NULL;
        const char* pass = NULL;
        
        user = imapSuccess ? lua_tolstring(L, 4, NULL) : NULL;
        imapSuccess = (user != NULL);
        
        pass = imapSuccess ? lua_tolstring(L, 5, NULL) : NULL;
        imapSuccess = (pass != NULL);
        
        connection = imapSuccess ? ImapConnection_Create(IMAP_SSL, ip, port, user, pass) : NULL;
        imapSuccess = (connection != NULL);
    }
    else if (imapSuccess) {
        connection = imapSuccess ? ImapConnection_Create(IMAP_NO_SSL, ip, port) : NULL;
        imapSuccess = (connection != NULL);
    }
    
    if (imapSuccess) {
        ImapConnection** connectionAddr = lua_newuserdata(L, sizeof(ImapConnection*));
        imapSuccess = (connectionAddr != NULL);
        
        if (imapSuccess) {
            *connectionAddr = connection;
            luaL_getmetatable(L, "lua_email_imap");
            lua_setmetatable(L, -2);
        }
    }
    
    if (imapSuccess) {
        //TODO: Andrei: Find logging mechanism.
        printf("IMAP connection created successfully ip=%s, port=%d\n", ip, port);
        return 1;
    }
    else {
        if (connection) {
            ImapConnection_Delete(connection);
        }
        return luaL_error(L, "Error creating imap connection");
    }
}

static int deleteImap(lua_State* L)
{
    ImapConnection** connectionAddr = NULL;
    
    connectionAddr = lua_touserdata(L, 1);
    ImapConnection_Delete(*connectionAddr);
    
    //TODO: Andrei: find logging mechanism
    printf("IMAP garbage collector called\n");
    
    return 0;
}


static int executeCommand(lua_State* L)
{
    ImapConnection** connectionAddr = NULL;
    const char* command = NULL;
    const char* output = NULL;
    int commandSuccess = 0;
    
    connectionAddr = luaL_checkudata(L, 1, "lua_email_imap");
    commandSuccess = (connectionAddr != NULL);
   
    command = commandSuccess ? lua_tolstring(L, 2, NULL) : NULL;
    commandSuccess = (command != NULL);
    
    output = commandSuccess ? ImapConnection_ExecuteCommand(*connectionAddr, command) : NULL;
    
    if (commandSuccess && output) {
        lua_pushstring(L, output);
        return 1;
    }
    else if (commandSuccess && output == NULL) {
        lua_pushnil(L);
        lua_pushfstring(L, "%s command is not valid for this connection", command);
        return 2;
    }
    else if (commandSuccess == 0) {
        return luaL_error(L, "Error executing IMAP command");
    }
}

static const luaL_Reg smtpFunctions[] = {
    {"sendEmail", sendEmail},
    {NULL, NULL}
};

static const luaL_Reg imapFunctions[] = {
    {"executeCommand", executeCommand},
    {NULL, NULL}
};

static const luaL_Reg emailLibrary[] = {
    {"makeSmtp", makeSmtp},
    {"makeImap", makeImap},
    {NULL, NULL}
};

/**
 * \brief
 * Creates a metatable with the given name and sets itself as
 * the __index field.
 * Also, sets gcCallback as the __gc field.
 * Leaves the metatable on top of the stack inside the given lua state.
 */
static void createMetatable(lua_State* L, const char* name, lua_CFunction gcCallback)
{
    luaL_newmetatable(L, name);
    
    //set metatable as the index table for itself.
    //in this way, when a function is called for the userdate,
    //it will be looked up in the metatable instead.
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    //insert gcCallback at the __gc index inside the metatable.
    //this function will be called by the garbage collector when
    //the user data needs to be removed.
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, gcCallback);
    lua_settable(L, -3);
}

int luaopen_lib_lua_email(lua_State* L)
{
    createMetatable(L, "lua_email_smtp", deleteSmtp);    
    //put the smtp functions in the metatable
    luaL_setfuncs(L, smtpFunctions, 0);
    
    createMetatable(L, "lua_email_imap", deleteImap);
    luaL_setfuncs(L, imapFunctions, 0);
    
    //register the emailLibrary functions in a table
    //return the table to the interpreter.
    luaL_newlib(L, emailLibrary);
    return 1;
}
