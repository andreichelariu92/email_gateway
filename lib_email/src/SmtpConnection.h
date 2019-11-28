#ifndef SmtpConnection_INCLUDE_GUARD
#define SmtpConnection_INCLUDE_GUARD

#include "Email.h"

struct _SmtpConnection;
typedef struct _SmtpConnection SmtpConnection;

extern const int SMTP_SSL;
extern const int SMTP_NO_SSL;

/**
 * \fn SmtpConnection_Create
 *
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

/**
 * \fn SmtpConnection_Delete
 *
 * \brief
 * Frees up the resources assoicated with the smtp connection.
 */
void SmtpConnection_Delete(SmtpConnection* c);

/**
 * \fn SmtpConnection_SendEmail
 *
 * \brief
 * Sends the email using the the smtp connection.
 *
 * \return
 * 1 in case of success; 0 in case of error.
 */
int SmtpConnection_SendEmail(SmtpConnection* c, Email* e);
#endif
