#include "headhsock.h"

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len);
void tv_sub(struct timeval *out, struct timeval *in);
                
int main(int argc, char *argv[])
{
	int len, sockfd;
	struct sockaddr_in ser_addr;
	char **pptr;
	struct hostent *sh;
      	struct in_addr **addrs;
        FILE *fp;

	if (argc!= 2)
	{
		printf("parameters not match.");
		exit(0);
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             
		printf("error when gethostbyname");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);            
	if (sockfd<0)
	{
		printf("error in socket");
		exit(1);
	}

	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);			
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);

        if ((fp = fopen("myfile.txt","r+t")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
       
	str_cli1(fp, sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in), &len);   

	close(sockfd);
        fclose(fp);
	exit(0);
}

void str_cli1(FILE *fp, int sockfd, struct sockaddr *addr, int addrlen, int *len)
{       
        long lsize, ci;
        char *buf;
        char sends[DATALEN2];
        float time_inv = 0.0;
        struct ack_so ack;
        int n, slen,length, ack_n = 0;
        struct timeval sendt, recvt;
	int check = 0;

        ci = 0;   
        fseek(fp,0,SEEK_END);
        lsize = ftell(fp);
        rewind(fp);
        buf = (char *) malloc (lsize);
	if (buf == NULL) exit(2);
        fread(buf,1,lsize,fp);
        buf[lsize] = '\0';
        gettimeofday(&sendt, NULL);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("The packet length 1 is %d bytes\n", DATALEN);
	printf("The packet length 2 is %d bytes\n", DATALEN2);

        while(ci<= lsize)
        {  
		if(check == 0) 
		{        
			length = DATALEN;
			check = 1;	
		}
	      	else 
		{
			length = DATALEN2;
			check = 0;
       		}
           	if((lsize+1-ci) <= length) 
		{
               		slen = lsize+1-ci;
           	}
           	else {
               		slen = length;
           	}

           	memcpy(sends, (buf+ci) , slen);
           	n = sendto(sockfd, &sends, slen, 0, addr, addrlen);
		ci += slen;
	        //while((ack_n = recvfrom(sockfd,&ack,2,0,addr,0) == 0));
		recvfrom(sockfd,&ack,2,0,addr,0);
		if (ack.num == 1|| ack.len == 0)
		{
			printf("Ack received!\n");
		} 	        
		printf("Bytes sent = %d\n ",n);                     
        }

        gettimeofday(&recvt,NULL);
        tv_sub(&recvt, &sendt);
        time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
        printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n ",time_inv,ci,(ci/(float)time_inv));               	                        	
}
void tv_sub(struct timeval *out, struct timeval *in)
{
        if((out->tv_usec -= in->tv_usec) <0)
        {
           --out ->tv_sec;
           out ->tv_usec += 1000000;
        }
        out->tv_sec -= in->tv_sec;
}
