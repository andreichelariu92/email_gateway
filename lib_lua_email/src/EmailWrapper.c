#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "lib_email/src/SmtpConnection.h"

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
    
    if (sendSuccess) {
        sendSuccess = SmtpConnection_SendEmail(*connectionAddr,
                                               sender,
                                               receiver,
                                               subject,
                                               content
                                              );
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

static const luaL_Reg smtpFunctions[] = {
    {"sendEmail", sendEmail},
    {NULL, NULL}
};

static const luaL_Reg emailLibrary[] = {
    {"makeSmtp", makeSmtp},
    {NULL, NULL}
};

int luaopen_lib_lua_email(lua_State* L)
{
    luaL_newmetatable(L, "lua_email_smtp");
    
    //set metatable as the index table for itself.
    //in this way, when a function is called for the userdate,
    //it will be looked up in the metatable instead.
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, deleteSmtp);
    lua_settable(L, -3);
    
    //put the smtp functions in the metatable
    luaL_setfuncs(L, smtpFunctions, 0);
    
    //register the emailLibrary functions in a table
    //return the table to the interpreter.s
    luaL_newlib(L, emailLibrary);
    return 1;
}