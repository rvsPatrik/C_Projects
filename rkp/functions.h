#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

void showVersion();
void showHelp();
void alap(int *senderMode,int *useSocket, int argc,char *argv[0]);
int Measurement(int **Values);
void BMPcreator(int *Values, int NumValues);
int FindPID();
void ReceiveViaFile();
void SendViaFile(int *data, int dataCount);
void SendViaSocket(int *data, int dataCount);
void ReceiveViaSocket();
void SignalHandler(int signal_code);




#endif // FUNCTIONS_H_