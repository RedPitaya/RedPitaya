/**
 * $Id: $
 *
 * @brief A simple Test SCPI client. Can send one pre-defined message and read back one answer :)
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <arpa/inet.h> 

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 
    bool binary_mode = false;

    if(argc != 3 && argc != 4)
    {
        printf("\n Usage: %s <ip of server> <SCPI command> <argument>\n",argv[0]);
        printf("\t Arguments:\n");
        printf("\t\t -b Binary mode\n");
        return 1;
    } 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    if (argc == 4 && strcmp(argv[3], "-b") == 0)
    {
        // Switch to bin mode
        strcpy(recvBuff, "ACQ:DATA:FORMAT BIN\r\n");

        if (send(sockfd, recvBuff, strlen(recvBuff), 0) == -1) {
            perror("send");
        }

      
   //strcpy(recvBuff, "SOUR:DIG:DATA:BIT LED4,1;\r\nSOUR:DIG:DATA:BIT LED5,1;\r\nSOUR:DIG:DATA:BIT LED6,1;\r\n");

    strcpy(recvBuff,
            /* LED ON */
            "DIG:PIN LED4,1;\r\nDIG:PIN LED5,1;\r\nDIG:PIN LED6,1;\r\nDIG:PIN:DIR INP,DIO1_P;\r\n"
                    /* ANALOG WRITE */
                    "ANALOG:PIN AOUT0,1;\r\nANALOG:PIN AOUT1,1.5;\r\nANALOG:PIN AOUT2,0.75;\r\nANALOG:PIN AOUT3,0.12345;\r\n"
                    /* ANALOG READ */
                    "ANALOG:PIN? AOUT0;\r\nANALOG:PIN? AOUT1;\r\nANALOG:PIN? AOUT2;\r\nANALOG:PIN? AOUT3\r\n");

    strcpy(recvBuff, argv[2]);
    strcat(recvBuff, "\r\n");
    if (send(sockfd, recvBuff, strlen(recvBuff), 0) == -1) {
        perror("send");
    }


   printf("Sent message %s\n", recvBuff	);	
   fputs("Received: ", stdout);
   fflush(stdout);

    while ( (n = (int) read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
    {
        recvBuff[n] = 0;
	    fputs("Received: ", stdout);
        if(fputs(recvBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
	    fflush(stdout);

    } 

    if(n < 0)
    {
        printf("\n Read error \n");
    } 

    return 0;
}
