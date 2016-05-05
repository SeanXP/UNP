/****************************************************************
  Copyright (C) 2016 Sean Guo. All rights reserved.

  > File Name:         < getservbyname.c >
  > Author:            < Sean Guo >
  > Mail:              < iseanxp+code@gmail.com >
  > Created Time:      < 2016/05/05 >
  > Description:        getservbyname() - get service entry

  struct servent * getservbyname(const char *name, const char *proto);

  struct  servent {
      char    *s_name;        // official name of service
      char    **s_aliases;    // alias list
      int     s_port;         // port service resides at
      char    *s_proto;       // protocol to use
  };

  ----

  struct servent * getservbyport(int port, const char *proto);

 ****************************************************************/

#include <stdio.h>
#include <netdb.h>          // getservbyname()

void print_servent(struct servent *ptr);

int main(int argc, char* argv[])
{
    struct servent * sptr;

    // getservbyport()
    sptr = getservbyname("domain",  "udp");
    print_servent(sptr);

    sptr = getservbyname("ftp",  "tcp");
    print_servent(sptr);

    sptr = getservbyname("ftps",  "udp");
    print_servent(sptr);

    // getservbyport()
    sptr = getservbyport(htons(21), "udp");
    print_servent(sptr);

    sptr = getservbyport(htons(22), "tcp");
    print_servent(sptr);

    sptr = getservbyport(htons(23), "tcp");
    print_servent(sptr);

    sptr = getservbyport(htons(53), "tcp");
    print_servent(sptr);

    // same port, different service
    sptr = getservbyport(htons(514), "tcp");
    print_servent(sptr);
    sptr = getservbyport(htons(514), "udp");
    print_servent(sptr);

    return 0;
}

void print_servent(struct servent *ptr)
{
    char *alias = NULL;
    if(ptr != NULL)
    {
        printf("\nname :\t\t%s\n", ptr->s_name);
        printf("port :\t\t%d\n", ntohs(ptr->s_port));
        printf("proto:\t\t%s\n", ptr->s_proto);
        for(alias = ptr->s_aliases[0]; alias != NULL; alias++)
            printf("s_aliases:\t\t%s\n", alias);
    }
}
