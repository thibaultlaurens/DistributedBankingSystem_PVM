/* 
 * File:   bankaccount.cpp
 * Author: thibault
 */

#include "pvminc.h"
#include "IOperation.h"
#include "IMessage.h"
#include "config.h"

//Bank account log file
FILE *op;

class BankAccount {
    
private:
    int mytid, ptid, amount, success;
    
    //send the amount the his parent task for a given message tag
    void SendAmountToParent(int msgtag){
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&amount, 1, 1);
        pvm_pkint(&success, 1, 1);
        pvm_send(ptid, msgtag);
        success = 0;
    }
    
public:

    BankAccount(int amount) {
        success = 0;
        //init a pvm session and set the bank account task id
        mytid = pvm_mytid();
        if (mytid < 0) {
            fprintf(op, "an error occured: mytid=%d\n", mytid);
            fclose(op);
            pvm_exit();
        }
        //get and set the parent task id
        ptid = pvm_parent();
        if ((ptid < 0) && (ptid == PvmNoParent)) {
            fprintf(op, "an error occured: parenttid=%d\n", ptid);
            fclose(op);
            pvm_exit();
        }
        //set the amount
        this->amount = amount;
        //send back the amount to the client to confirm that everything is OK
        SendAmountToParent(BA_INIT_CB);
    }
	
	void WithdrawMoney(int amount) {
        //check if the amount will be > 0
        if (this->amount - amount > 0) {
            this->amount -= amount;
        }
        else
            success = 1;
        //if not, do nothing and return the amount
        SendAmountToParent(BA_OPERATION_CB);
    }

    void AddMoney(int amount) {
        this->amount += amount;
        SendAmountToParent(BA_OPERATION_CB);
    }
	
    void GetAmount() {
        SendAmountToParent(BA_OPERATION_CB);
    }
    
};


int main() {

    int operationId, amount;
    bool exit = false;

    op = fopen("bout", "w");
    fprintf(op, "Bankaccount log file\n\n");
    
    //wait for a headoffice init message
    pvm_recv(-1, BA_INIT);
    pvm_upkint(&amount, 1, 1);
    //instanciate and init an account with an amount received from the headoofice
    BankAccount account(amount);

    while (exit == false) {
        pvm_recv(-1, BA_OPERATION);
        pvm_upkint(&operationId, 1, 1);

        //the first int received redirects to the bankaccount operations in IOperation.H
        switch (operationId) {
            case ADDMONEY:
                pvm_upkint(&amount, 1, 1);
                account.AddMoney(amount);
                break;
            case WITHDRAWMONEY:
                pvm_upkint(&amount, 1, 1);
                account.WithdrawMoney(amount);
                break;
            case GETAMOUNT:
                account.GetAmount();
                break;
            case EXITB:
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







