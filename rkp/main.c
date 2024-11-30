#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    srand(time(NULL));

    int senderMode = 0;
    int useSocket = 0; 
    

    alap(&senderMode, &useSocket, argc, argv);
    if ( senderMode<0 || useSocket<0 ){
        return EXIT_FAILURE;
    }
    
    signal(SIGINT,SignalHandler);
 
    signal(SIGUSR1, SignalHandler);

    int *Values = NULL;
    int size = Measurement(&Values);
   
    
    if ( senderMode==0&&useSocket==0 ){
        SendViaFile(Values,size);
        printf("sending file");
        free(Values);
        exit(0);
     

    }else if( senderMode==0&&useSocket==1 ){
        SendViaSocket(Values,size);
        free(Values);
        exit(0);
    }

    
    else if ( senderMode == 1&&useSocket == 0){
        while(1){
            ReceiveViaFile();
            pause();
        }
        
        
    }else if( senderMode == 1&&useSocket==1){
        ReceiveViaSocket();
    }
    



    return 0;
}