#ifndef SmtpConnection_INCLUDE_GUARD
#define SmtpConnection_INCLUDE_GUARD

#include "Email.h"

struct _SmtpConnection;
typedef struct _SmtpConnection SmtpConnection;

//TODO: Andrei: Add/generate doxygen documentation.

extern const int SMTP_SSL;
extern const int SMTP_NO_SSL;

/**
 * \brief
 * Create a SMTP connection based on the following options:
 * SMTP_SSL = input parameters are:
 *            const char* address and int port
 * SMTP_NO_SSL = input parameters are:
 *               const char* address, int port,
 *               const char* user, const char* password
 * \return
 * Returns a pointer to a SmtpConnection or NULL in case of
 * error.
 */
SmtpConnection* SmtpConnection_Create(int option, ...);
void SmtpConnection_Delete(SmtpConnection* c);
int SmtpConnection_SendEmail(SmtpConnection* c,
    const char* from,
    const char* to,
    const char* subject,
    const char* content);
#endif
