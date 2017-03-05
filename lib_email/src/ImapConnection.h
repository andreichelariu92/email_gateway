#ifndef ImapConnection_INCLUDE_GUARD
#define ImapConnection_INCLUDE_GUARD

struct _ImapConnection;
typedef struct _ImapConnection ImapConnection;

extern const int IMAP_SSL;
extern const int IMAP_NO_SSL;
extern const int IMAP_SSL_DEFAULT_PORT;
extern const int IMAP_NO_SSL_DEFAULT_PORT;

/**
 * \brief
 * Create IMAP connection.
 * option = IMAP_SSL => const char* ip, int port
 *                      const char* user, const char* pass
 * option = IMAP_NO_SSL => const char* ip, int port
 * 
 * \return
 * Opaque pointer to an ImapConnection, zero in case of error.
 */
ImapConnection* ImapConnection_Create(int option, ...);

/**
 * \brief
 * Destructor for IMAP connection.
 */
void ImapConnection_Delete(ImapConnection* c);

/**
 * \brief
 * Executes the given command for the specified connection.
 * 
 * \return
 * The result of the command. If the command fails, it returns NULL.
 * Empty string("") is a valid result!
 */
const char* ImapConnection_ExecuteCommand(ImapConnection* connection, const char* command);
#endif