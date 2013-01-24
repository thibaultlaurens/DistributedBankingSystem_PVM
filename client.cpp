/* 
 * File:   client.cpp
 * Author: thibault
 */

#include "pvminc.h"
#include "IOperation.h"
#include "IMessage.h"
#include "config.h"

int nstid[0];

class Client {
    
private:
    int mytid, headoperation;
    int nstid[0], banktids[10], accounttids[3][10];

    //ask and receive a bank task id from the nameserver
    int AskHeadOffice(int nstid[0]) {
        int banktid;
        headoperation = INIT;

        //ask the nameserver a headoffice servers
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&headoperation, 1, 1);
        pvm_send(nstid[0], NS_OPERATION);

        //receive headoffice1 taskid
        pvm_recv(-1, NS_OPERATION_CB);
        pvm_upkint(&banktid, 1, 1);

        return banktid;
    }

    //ask and receive a bankaccount task id from a headoffice server and return it
    int AskBankAccount(int banktid, int amount) {
        int accounttid;
        //ask headoffice for bankaccounts
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&headoperation, 1, 1);
        pvm_pkint(&amount, 1, 1);
        pvm_send(banktid, HO_OPERATION);

        //receive bankaccount 1 taskid
        pvm_recv(-1, HO_OPERATION_CB);
        pvm_upkint(&accounttid, 1, 1);
        pvm_upkint(&amount, 1, 1);

        return accounttid;
    }

    //ask the user to select a bank account and call SelectOperation(bankInfo, bankAccountInfo)
    void SelectBankAccountInfo(int bankInfo) {
        int bankAccountInfo;

        printf("BANKACCOUNT\n\t");
        printf("0- account:%d\n\t", accounttids[bankInfo][0]);
        printf("1- account:%d\n\t", accounttids[bankInfo][1]);
        printf("2- return to bank listing\n");

        scanf("%d", &bankAccountInfo);

        if (bankAccountInfo == 2) {
            SelectBank();
        } else if (bankAccountInfo == 0 || bankAccountInfo == 1) {
            SelectOperation(bankInfo, bankAccountInfo);
        }
    }

    //ask the user to select an operation to perform and redirect to the right method to perform the operation
    int SelectOperation(int bankInfo, int bankAccountInfo) {

        int actionInfo, amount, success;

        //operation are stored in bankaccount.h
        printf("OPERATION\n\t");
        for (int i = 0; i != GETAMOUNT; i++) {
            printf("%d %s\n\t", i + 1, operations[i]);
        }
        printf("4 TRANSFER\n\t");
        printf("5 EXIT\n\t");
        printf("6 return to bank account listing\n");
        scanf("%d", &actionInfo);

        headoperation = BANKACCOUNTOP;
        if (actionInfo == 1 || actionInfo == 2) {

            printf("What is the amount ?\n");
            scanf("%d", &amount);

            PreSendOperation(bankInfo, bankAccountInfo, actionInfo, amount);
            pvm_send(banktids[bankInfo], HO_OPERATION);

            amount = ReceiveOperation(bankInfo, bankAccountInfo);
            pvm_upkint(&success, 1, 1);
            if(success == 0)
                printf("\tamount:%d\n", amount);
            else
                printf("\terror - not enough money on the account - amount:%d\n", amount);

        } else if (actionInfo == 3) {
            
            PreSendOperation(bankInfo, bankAccountInfo, actionInfo);
            pvm_send(banktids[bankInfo], HO_OPERATION);
            
            amount = ReceiveOperation(bankInfo, bankAccountInfo);
            printf("\tamount: %d\n", amount);
        } else if (actionInfo == 5){
            Exit();
        } else if (actionInfo == 6) {
            SelectBankAccountInfo(bankInfo);
        } else if (actionInfo == 4){
            BankTransfer(bankInfo, bankAccountInfo);
        }

        SelectOperation(bankInfo, bankAccountInfo);
    }
    
	//perform a "withdraw" on the bankaccount parameter, if OK perform an "add" on asked bankaccount and/or bank 
    void BankTransfer(int bankInfo, int bankAccountInfo){
	
        int tobank, toaccount, amount, fromamount, toamount, success;
        printf("Insert bank id, bank account id and amount\n");
        scanf("%d %d %d",&tobank, &toaccount, &amount);
        //try to withdraw money "from" the current bank account
        PreSendOperation(bankInfo, bankAccountInfo, WITHDRAWMONEY, amount);
        pvm_send(banktids[bankInfo], HO_OPERATION);
        fromamount = ReceiveOperation(bankInfo, bankAccountInfo);
        pvm_upkint(&success, 1, 1);
        //Withdraw ok, put the money into the "To" account
        if(success == 0){
                PreSendOperation(tobank, toaccount, ADDMONEY, amount);
                pvm_send(banktids[tobank], HO_OPERATION);
                toamount = ReceiveOperation(tobank, toaccount);
                printf("\tsuccess - %d were transfered from bank %d account %d to bank %d account %d\n",amount, bankInfo, bankAccountInfo, tobank, toaccount);
        }
        else
                printf("\terror - not enough money on the account - amount:%d\n", fromamount);
    }
    
    //pack bank account and actionInfo for an headoffice call
    void PreSendOperation(int bankInfo, int bankAccountInfo, int actionInfo){
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&headoperation, 1, 1);
        pvm_pkint(&accounttids[bankInfo][bankAccountInfo], 1, 1);
        pvm_pkint(&actionInfo, 1, 1);
    }
    
     //pack bank account and actionInfo for an headoffice call, overload with the amount information
    void PreSendOperation(int bankInfo, int bankAccountInfo, int actionInfo, int amount){
        PreSendOperation(bankInfo, bankAccountInfo, actionInfo);
        pvm_pkint(&amount, 1, 1);
    }
    
    //wait to receive callback operation from the headoffice
    int ReceiveOperation(int bankInfo, int bankAccountInfo){
        int amount;
        pvm_recv(-1, HO_OPERATION_CB);
        pvm_upkint(&accounttids[bankInfo][bankAccountInfo], 1, 1);
        pvm_upkint(&amount, 1, 1);
        return amount;
    }
    
	//call SendExit() for the 4 bank accounts, the 2 banks, the ns and then pvm_exit()
    void Exit(){
        //Exit the 4 bank accounts
        SendExit(EXITB, BA_OPERATION, accounttids[0][0]);
        SendExit(EXITB, BA_OPERATION, accounttids[0][1]);
        SendExit(EXITB, BA_OPERATION, accounttids[1][0]);
        SendExit(EXITB, BA_OPERATION, accounttids[1][1]);
        
        //Exit the 2 banks
        SendExit(EXITH, HO_OPERATION, banktids[0]);
        SendExit(EXITH, HO_OPERATION, banktids[1]);
        
         //Exit the nameserver
        SendExit(EXITNS, NS_OPERATION, nstid[0]);
        
        //then the client
        pvm_exit();
    }
    
	//send the operation parameter, on the msgtag parameter, to the processId parameter 
    void SendExit(int operation, int msgtag, int processtId){
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&operation, 1, 1);
        pvm_send(processtId, msgtag);
    }

public:
	//init a pvm session and return the task id
    Client() {
        mytid = pvm_mytid();
        if (mytid < 0) {
            printf("an error occured - client init\n");
        }
        printf("*info* mytid=%d\n", mytid);        
    }

	//create 2 headoffices with a ns tid and 2 bank accounts with fixed amount for each headoffice
    void BootStrapper(int nstid[0]) {
        
        //create 2 headoffices with a given nameserver task id
        banktids[0] = AskHeadOffice(nstid);
        banktids[1] = AskHeadOffice(nstid);

        //create 2 bank accounts with random amount for these headoffice
        accounttids[0][0] = AskBankAccount(banktids[0], 211);
        accounttids[0][1] = AskBankAccount(banktids[0], 157);
        accounttids[1][0] = AskBankAccount(banktids[1], 184);
        accounttids[1][1] = AskBankAccount(banktids[1], 126);

        printf("*info* bootsrapper OK - 2 headoffices and 4 bankaccounts created\n");
    }

    //Ask the user to select a bank and call SelectBankAccountInfo(bankInfo)
    void SelectBank() {
        int bankInfo;

        printf("BANK:\n\t");
        printf("0- bank:%d\n\t", banktids[0]);
        printf("1- bank:%d\n", banktids[1]);

        scanf("%d", &bankInfo);

        SelectBankAccountInfo(bankInfo);
    }
};

int main() {

    //create a client
    Client client;

    //spawn the nameserver, get its taskid
    pvm_spawn(nameServerExe, (char**) 0, PvmTaskHost, nameServer, 1, nstid);

    client.BootStrapper(nstid);

    //start user interaction - first step, select a bank
    client.SelectBank();
    
    return 0;
}

