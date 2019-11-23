#ifndef Email_INCLUDE_GUARD
#define Email_INCLUDE_GUARD

struct _Email;
typedef struct _Email Email;

/**
 * \fn Email_Create
 *
 * \brief
 * Create an email structure, containing the necessary data for
 * an email to be sent.
 *
 * \return
 * Pointer to email structure, or 0 in case of error.
 */
Email* Email_Create(const char* sender,
        const char* receiver,
        const char* subject,
        const char* content);

/**
 * \fn Email_GetSender
 *
 * \brief
 * Getter for sender of an email.
 *
 * \attention
 * Don't free the pointer. To modify the sender, create a new email.
 *
 * \return
 * Pointer to characters containing the sender
 */
char* Email_GetSender(Email* e, const char* option);

/**
 * \fn  Email_GetReceiver
 *
 * \brief
 * Getter for receiver of email.
 *
 * \attention
 * Similiar with Email_GetSender.
 *
 * \return
 * Pointer to characters containing the receiver.
 */
char* Email_GetReceiver(Email* e, const char* option);

/**
 * \fn Email_GetSubject
 *
 * \brief
 * Getter for subject of email.
 *
 * \attention
 * Similar with Email_GetSubject.
 *
 * \return
 * Pointer to characters containing the subject.
 */
char* Email_GetSubject(Email* e, const char* option);

/**
 * \fn  Email_GetContent
 *
 * \brief
 * Getter for content of email.
 *
 * \attention
 * Similar with Email_GetSubject.
 *
 * \return
 * Pointer to characters containing the content of the email.
 */
char* Email_GetContent(Email* e, const char* option);

/**
 * \fn Email_Delete
 *
 * \brief
 * Method that frees the resources associated with the email structure.
 */
void Email_Delete(Email* e);
#endif
