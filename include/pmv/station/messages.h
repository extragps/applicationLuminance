#ifndef MESSAGES_H_
#define MESSAGES_H_

void messagesInit(void);
void messagesTerm(void);
BscLigne *messageGetByNumber(int numero);
BscLigne *messageGetMessTestByNumber(int numero);

#endif /*MESSAGES_H_*/
