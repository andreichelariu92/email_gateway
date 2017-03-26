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
    int argIndex = 1;
    const char* ip = NULL;
    int port = 0;
    int smtpSuccess = 0;
    SmtpConnection* connection = NULL;
    
    sslFlag = lua_toboolean(L, argIndex++);
    
    ip = lua_tolstring(L, argIndex++, NULL);
    smtpSuccess = (ip != NULL);
    
    port = smtpSuccess ? lua_tointegerx(L, argIndex++, NULL) : 0;
    smtpSuccess = (port > 0);
    
    if (sslFlag && smtpSuccess) {
        const char* user = NULL;
        const char* pass = NULL;
        
        user = lua_tolstring(L, argIndex++, NULL);
        smtpSuccess = (user != NULL);
        
        pass = smtpSuccess ? lua_tolstring(L, argIndex++, NULL) : NULL;
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