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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>


void showVersion() {
    printf("\nA program verzioszama: v1.0\n");
    printf("A program elkeszulesenek datuma: 2024.05.02");
    printf("A program fejlesztojenek neve: Revesz Patrik\n");
}

void showHelp() { 
    printf("A program uzemmpdjai: \n");
    printf("- A program alapertelmezetten kuldokent viselkedik -\n");
    printf("\n| -send | - A program kuldokent viselkedik.\n");
    printf("| -receive | - A program fogadokent mukodik.\n");
    printf("\nA kommunikacio modja:\n");
    printf("- A program alapertelmezett kommunikacios modja: fajl -\n");
    printf("| -file | - A program fajt hasznal kommunikaciora.\n");
    printf("| -socket | - A program socketet hasznal a kommunikaciora.\n");
}

void alap(int *senderMode,int *useSocket, int argc,char *argv[0]){

    if ( strcmp(argv[0],"./chart") != 0){
        fprintf(stderr,"Hiba a program nevében");
        exit(EXIT_FAILURE);
    }
    if (argc == 1) {
        showHelp();
        exit(EXIT_SUCCESS);
    }
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            showHelp();
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "--version") == 0) {
            showVersion();
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-send") == 0) {
            *senderMode = 0;
        } else if (strcmp(argv[i], "-receive") == 0) {
            *senderMode = 1;
        } else if (strcmp(argv[i], "-file") == 0) {
            *useSocket = 0;
        } else if (strcmp(argv[i], "-socket") == 0) {
            *useSocket = 1;
        } else {
            printf("Nem érvényes argumentum: %s\n", argv[i]);
            showHelp();
            exit(EXIT_FAILURE);
        }
    }  
}

int Measurement(int **Values)
{
    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);

    int minutes = localTime->tm_min;
    int seconds = localTime->tm_sec;
    int recordnum = 100;
    if (minutes < 15) {
        recordnum = (minutes * 60) + seconds + 100;
    } else if (minutes < 30) {
        recordnum = ((minutes - 15) * 50) + seconds + 100;
    } else if (minutes < 45) {
        recordnum = ((minutes - 30) * 60) + seconds + 50;
    } else {
        recordnum = ((minutes - 45) * 70) + seconds;
    }

    srand(time(NULL));

    *Values = (int *)malloc(recordnum * sizeof(int));

    (*Values)[0] = 0;
    double randomValue;
    for (int i = 1; i < recordnum; i++) {
         randomValue = (double)rand() / RAND_MAX;
        if (randomValue < 0.4) {
            (*Values)[i] = (*Values)[i - 1] + 1; 
        } else if (randomValue < 0.75) {
            (*Values)[i] = (*Values)[i - 1] - 1; 
        } else {
            (*Values)[i] = (*Values)[i - 1]; 
        }
    }

    return recordnum;
}






#define BMP_HEADER_SIZE 62

void BMPcreator(int *Values, int NumValues)
{

    unsigned long int fsize = BMP_HEADER_SIZE + (NumValues * NumValues * 3);
    int file = open("color_chart.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH);


    uint8_t f_header[BMP_HEADER_SIZE] = {
        0x42, 0x4D, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0x36, 0x00, 0x00, 0x00, 
        0x28, 0x00, 0x00, 0x00, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0x01, 0x00, 
        0x18, 0x00, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0  
    };

    *((uint32_t *)&f_header[2]) = fsize;
    *((uint32_t *)&f_header[18]) = NumValues;
    *((uint32_t *)&f_header[22]) = NumValues;

    write(file, f_header, BMP_HEADER_SIZE);

    uint8_t **pixels = (uint8_t **)calloc(NumValues, sizeof(uint8_t *));
    for (int i = 0; i < NumValues; ++i)
    {
        pixels[i] = (uint8_t *)calloc(NumValues * 3, sizeof(uint8_t)); 
    }

    for (int i = 0; i < NumValues; ++i)
    {
        int index = (NumValues / 2) + Values[i];
        if (index < 0) {
            index = 0;
        }
        if (index >= NumValues) {
            index = NumValues - 1;
        }

        pixels[index][i * 3] = 0xFF;  
        pixels[index][i * 3 + 1] = 0x00;  
        pixels[index][i * 3 + 2] = 0x00;  
    }

    for (int i = 0; i < NumValues; ++i)
    {
        write(file, pixels[i], NumValues * 3); 
    }

    for (int i = 0; i < NumValues; ++i)
    {
        free(pixels[i]);
    }
    free(pixels);
    close(file);
}



int FindPID()
{
    DIR *proc_dir, *proc_dir2;
    struct dirent *proc_entry, *proc_entry2;
    char line[100];
    FILE *status_file;
    
    int found_pid = 0;

    const char *target_process = "Name:\tchart\n"; 
    const char *pid_indicator = "Pid:"; 

    proc_dir = opendir("/proc");
    char array[4];
    if (proc_dir == NULL) {
        perror("Failed to open /proc");
        return -1;
    }

    while(( proc_entry = readdir(proc_dir)) != NULL ) {
        if (isdigit(proc_entry->d_name[0])) { 
            char *path = (char *)malloc(strlen(proc_entry->d_name)+5);
            sprintf(path,"%s/%s","/proc",proc_entry->d_name);

            proc_dir2=opendir(path);
            while(( proc_entry2 = readdir(proc_dir2)) != NULL ){
                if(( strcmp(proc_entry2->d_name,"status" ) == 0 )){
                    char *path2 = (char *)malloc(strlen(proc_entry2->d_name)+5);
                    sprintf(path2,"%s/%s",path,"status");
                    status_file = fopen(path2,"r");
                    if( status_file == NULL ){
                        return -1;
                    }
                    while( fgets(line,sizeof(line),status_file) != NULL ){
                        if( strcmp(line,target_process) == 0 ){
                            found_pid = 1;
                        }
                        if (( strncmp(line,pid_indicator,4)==0)&&(found_pid==1 )){
                            memcpy(array,&line[5],4);
                            closedir(proc_dir);
                            closedir(proc_dir2);



                            return atoi(array);
                        }
                    }
                }
            }


            closedir(proc_dir2);
        }
    }

    closedir(proc_dir);
    return -1;
}

        
void ReceiveViaFile() {
    const char *filename = "/Measurement.txt";
    const char *home_directory = getenv("HOME");

    if (home_directory == NULL) {
        fprintf(stderr, "Home directory not found\n");
        exit(EXIT_FAILURE);
    }

    char *file_path = malloc(strlen(home_directory) + strlen(filename) + 1);
    if (file_path == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    strcpy(file_path, home_directory); 
    strcat(file_path, filename); 

    FILE *input_file = fopen(file_path, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Error opening file\n");
        free(file_path);
        exit(EXIT_FAILURE);
    }

    char line[100]; 
    int count = 0;
    int *values = (int *)malloc(sizeof(int)); 

    if (values == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        fclose(input_file);
        free(file_path);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, sizeof(line), input_file) != NULL) {
        values[count] = atoi(line); 
        ++count;
        int *new_values = realloc(values, sizeof(int) * (count + 1)); 
        if (new_values == NULL) {
            fprintf(stderr, "Memory allocation error during realloc\n");
            free(values); 
            fclose(input_file);
            free(file_path);
            exit(EXIT_FAILURE);
        }
        values = new_values;
    }

    fclose(input_file);
    
   
    printf("Creating BMP file...\n");
    BMPcreator(values, count); 
    printf("BMP file creation complete.\n");

    free(values);
    free(file_path);
}


void SendViaFile(int *data, int dataCount) {

    const char *filename = "/Measurement.txt";
    const char *home_directory = getenv("HOME");

    if (home_directory == NULL) {
        fprintf(stderr, "Home directory not found\n");
        exit(EXIT_FAILURE);
    }

    char *file_path = malloc(strlen(home_directory) + strlen(filename) + 1);
    if (file_path == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }

    strcpy(file_path, home_directory); 
    strcat(file_path, filename); 



    FILE *outputFile = fopen(file_path, "w");
    if (outputFile == NULL) {
        perror("Error opening file");
        free(file_path);
        exit(EXIT_FAILURE);
    }


    for (int i = 0; i < dataCount; i++) {
        fprintf(outputFile, "%d\n", data[i]);
    }

    fclose(outputFile);
    free(file_path);


    pid_t receiverPid = FindPID(); 


    if (receiverPid == -1) {
        fprintf(stderr, "No process found in receiving mode\n");
        exit(EXIT_FAILURE);
    } else {
        kill(receiverPid, SIGUSR1); 


    }
}


void SendViaSocket(int *data, int dataCount)
{


    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); 

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3333);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socklen_t addr_len = sizeof(server_addr);


    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }


    int enable = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("Failed to set socket option");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    if (sendto(socket_fd, &dataCount, sizeof(int), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Error sending data count");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    signal(SIGALRM, SignalHandler);
    alarm(1); 

    int response = 0;


    ssize_t received_bytes = recvfrom(socket_fd, &response, sizeof(int), 0, (struct sockaddr *)&server_addr, &addr_len);
    if (received_bytes < 0) {
        perror("Error receiving response");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    signal(SIGALRM, SIG_IGN);


    if (response != dataCount) {
        fprintf(stderr, "Received size does not match expected size\n");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    if (sendto(socket_fd, data, dataCount * sizeof(int), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Error sending data");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }


    int sent_bytes = 0;
    received_bytes = recvfrom(socket_fd, &sent_bytes, sizeof(int), 0, (struct sockaddr *)&server_addr, &addr_len);
    if (received_bytes < 0 || sent_bytes != dataCount * sizeof(int)) {
        fprintf(stderr, "Mismatch in sent/received bytes\n");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    printf("Data successfully sent and confirmed\n");

    close(socket_fd); 
}

void ReceiveViaSocket()
{


    int sock_fd;
    int bytes_received;
    int recv_flag = 0;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_addr_len = sizeof(client_addr);
    int array_size;
    

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(3333);


    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }


    int enable = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));


    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }


    while (1) {
        printf("Waiting for incoming data...\n");


        bytes_received = recvfrom(sock_fd, &array_size, sizeof(int), recv_flag, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received < 0) {
            perror("Error receiving array size");
            exit(EXIT_FAILURE);
        }


        int *received_data = (int *)malloc(array_size * sizeof(int));
        if (!received_data) {
            fprintf(stderr, "Memory allocation failure\n");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        printf("Memory allocated for received data\n");


        int bytes_sent = sendto(sock_fd, &array_size, sizeof(int), recv_flag, (struct sockaddr *)&client_addr, client_addr_len);
        if (bytes_sent <= 0) {
            perror("Error sending allocation confirmation");
            free(received_data);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        printf("Sent allocation confirmation\n");


        bytes_received = recvfrom(sock_fd, received_data, array_size * sizeof(int), recv_flag, (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received < 0) {
            perror("Error receiving data");
            free(received_data);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        printf("Received data successfully\n");


        bytes_sent = sendto(sock_fd, &bytes_received, sizeof(int), recv_flag, (struct sockaddr *)&client_addr, client_addr_len);
        if (bytes_sent < 0) {
            perror("Error sending byte confirmation");
            free(received_data);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        printf("Sent byte confirmation\n");


        printf("Creating BMP file...\n");
        BMPcreator(received_data, array_size);
        printf("BMP file creation complete\n");


        free(received_data);
    }


    close(sock_fd);
}

void SignalHandler(int signal_code)
{
    switch (signal_code) {
        case SIGUSR1:
            printf("The \"File Transfer Service\" is currently unavailable...\n");
            break;
        
        case SIGINT:
            printf("Goodbye!\n");
            exit(EXIT_SUCCESS); 
            break;
        
        case SIGALRM:
            fprintf(stderr, "Server timeout, no response within the specified time\n");
            exit(EXIT_FAILURE); 
            break;
        
        default:
            printf("Unexpected signal received: %d\n", signal_code);
            break;
    }
}