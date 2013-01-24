/* 
 * File:   IOperation.h
 * Author: thibault
 */

#ifndef IOPERATION_H
#define	IOPERATION_H

//define bank account operation ids
const char operations[][20]={"ADDMONEY", "WITHDRAWMONEY","GETAMOUNT","EXIT"};
enum BankAccountOperation {ADDMONEY = 1, WITHDRAWMONEY = 2, GETAMOUNT = 3, EXITB = 100};

//define headoffice operation ids
enum HeadOperation {INIT = 1, BANKACCOUNTOP = 2, EXITH = 100};

//define nameserver operation ids
enum NameServerOperation {EXITNS = 100};

#endif	/* IOPERATION_H */

