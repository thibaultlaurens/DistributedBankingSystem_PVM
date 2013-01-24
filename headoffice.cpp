/* 
 * File:   bankaccount.cpp
 * Author: thibault
 */

#include "pvminc.h"
#include "IOperation.h"
#include "IMessage.h"
#include "config.h"

//headoffice log file
FILE *op;

class HeadOffice {
    
private:
    int mytId, ptId, clientId, accounttids[10];
    
public:

    HeadOffice(int clientId) {
        //init a pvm session and set the headoffice task id
        mytId = pvm_mytid();
        if (mytId < 0) {
            fprintf(op, "an error occured - init headoffice - mytid=%d\n", mytId);
            fclose(op);
            pvm_exit();
        }

        //get and set the parent task id
        ptId = pvm_parent();
        if ((ptId < 0) && (ptId == PvmNoParent)) {
            fprintf(op, "an error occured - get headoffice parent task id - ptId=%d\n", ptId);
            fclose(op);
            pvm_exit();
        }
        
        //set the client task id
        this->clientId = clientId;

    }

    void InitBankAccount(char* processName, char* serverName, int amount) {

        pvm_spawn(processName, (char**) 0, PvmTaskHost, serverName, 1, accounttids);

        //send the amount to the bankaccount
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&amount, 1, 1);
        pvm_send(accounttids[0], BA_INIT);

        //receive the amount from the bankaccount
        pvm_recv(-1, BA_INIT_CB);
        pvm_upkint(&amount, 1, 1);

        //send back accounts tids and amount to the client
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&accounttids[0], 1, 1);
        pvm_pkint(&amount, 1, 1);
        pvm_send(clientId, HO_OPERATION_CB);
    }

    void ProcessOperation() {
        int tIds[10], amount, operationId, success;

        pvm_upkint(&tIds[0], 1, 1);
        pvm_upkint(&operationId, 1, 1);
        pvm_upkint(&amount, 1, 1);

        //Send the operation to the bankaccount
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&operationId, 1, 1);
        pvm_pkint(&amount, 1, 1);
        pvm_send(tIds[0], BA_OPERATION);

        //receive the amount from the client
        pvm_recv(-1, BA_OPERATION_CB);
        pvm_upkint(&amount, 1, 1);
        pvm_upkint(&success, 1, 1);

        //send back the amount to the client
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&tIds[0], 1, 1);
        pvm_pkint(&amount, 1, 1);
        pvm_pkint(&success, 1, 1);
        pvm_send(clientId, HO_OPERATION_CB);
    }
};

int main() {
    int operationId, amount, clientId;
    bool exit;

    op = fopen("hout", "w");
    fprintf(op, "Headoffice log file\n\n");
    
    //wait for a nameserver init message
    pvm_recv(-1, HO_INIT);
    pvm_upkint(&clientId, 1, 1);

    //instanciate and init a headoffice with a clientId received from the nameserver
    HeadOffice office(clientId);

    while (exit == false) {
        pvm_recv(-1, HO_OPERATION);
        pvm_upkint(&operationId, 1, 1);
        
        //the first int received redirects to the headoffice operations in IOperation.H
        switch (operationId) {
            case INIT:
                //spawn bankaccount
                pvm_upkint(&amount, 1, 1);
                office.InitBankAccount(bankaccountExe, bankaccountServer, amount);
                break;
            case BANKACCOUNTOP:
                //launch a bankaccount execution
                office.ProcessOperation();
                break;
            case EXITH:
                exit = true;
                break;
                //If the first int received is not something from the enum, pvm quit
            default:
                exit = true;
                break;
        }
    }
    pvm_exit();
    return 0;
	
}