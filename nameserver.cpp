/* 
 * File:   nameserver.cpp
 * Author: thibault
 */

#include "pvminc.h"
#include "IOperation.h"
#include "IMessage.h"
#include "config.h"

//nameserver log file;
FILE *op; 

class NameServer {
private:
    int mytId, ptId, bankId[10];
public:
	//init a pvm session and set the nameserver task id
    NameServer() {
        
        mytId = pvm_mytid();
        if (mytId < 0) {
            fprintf(op, "an error occured - init nameserver - mytid=%d\n", mytId);
            fclose(op);
            pvm_exit();
        }

        //get the parent task id
        ptId = pvm_parent();
        if ((ptId < 0) && (ptId == PvmNoParent)) {
            fprintf(op, "an error occured - get nameserver parent task id - parenttid=%d\n", ptId);
            fclose(op);
            pvm_exit();
        }
    }

    void InitHeadOffice(char* processName, char* serverName) {
        //spawn a headoffice
        pvm_spawn(processName, (char**) 0, PvmTaskHost, serverName, 1, bankId);

        //give the clientId to the headoffice
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&ptId, 1, 1);
        pvm_send(bankId[0], HO_INIT);

        //send back headoffice tid to the client
        pvm_initsend(PvmDataDefault);
        pvm_pkint(&bankId[0], 1, 1);
        pvm_send(ptId, NS_OPERATION_CB);
    }
	
};

int main() {
    int operationId;
    bool exit = false;
    op = fopen("nout", "w");
    fprintf(op, "Nameserver log file\n\n");

    //Instantiate and init a nameserver
    NameServer nameServer;
    
    //inifinite loop
    while (exit == false) {
        pvm_recv(-1, NS_OPERATION);
        pvm_upkint(&operationId, 1, 1);
		//route the operationId to the right method
        switch (operationId) {
            case INIT:
                nameServer.InitHeadOffice(headofficeExe, headofficeServer);
                break;
            case EXITNS:
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

