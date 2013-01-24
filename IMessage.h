/* 
 * File:   IMessage.h
 * Author: thibault
 */

#ifndef IMESSAGE_H
#define	IMESSAGE_H

//describe the protocol used for message tag for each component

enum NameServerMessage{NS_OPERATION = 1, NS_OPERATION_CB = 2 };

enum HeadOfficeMessage{HO_INIT = 5, HO_OPERATION = 11, HO_OPERATION_CB = 12};

enum BankAccountMessage { BA_INIT = 1000, BA_OPERATION = 1001, BA_INIT_CB = 2000, BA_OPERATION_CB = 2001};

#endif	/* IMESSAGE_H */

