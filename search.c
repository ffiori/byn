/* This code is from Gonzalo Navarro, 1998.
   It can be freely used for academic teaching/learning and research
   purposes. Usage for direct or indirect commercial advantage is not
   allowed without written permission from the authors.
*/


#include "partexact.h"
#include "countcode.i"

extern uchar Map[256];

void partexactheur (int r, int m, int n, uchar **pattern, 
		    register uchar *text, int k, int c)

   { static PartExact P[10];
     int i,nn=n;
     static bool first = true;
     static int rp,ii;

     if (first)
        { for (i=0;i<256;i++) Map[i]=i;
          rp = r;
          if (c == 32)
             { if (m==30)
                  { if (k == 4) rp = 28;
                    else if (k == 8) rp = 20;
                  }
             }
          else if (c == 13)
             { if (m==30)
                  { if (k == 4) rp = 22;
                    else if (k == 8) rp = 16;
                  }
             }
	  i=0;
          while (i+2*rp <= r)
	     { prepPartExact (P+i/rp,pattern+i,rp,m,k);
	       i += rp;
	     }
	  ii = i/rp;
	  prepPartExact (P+ii,pattern+i,r-i,m,k);
	  first = false;
	}

     for (i=0;i<r;i++)
	 { strcpy (text+nn,pattern[i]);
	   nn += strlen(pattern[i]);
	 }
     text[nn]=1;
     
     for (i=0;i<=ii;i++) 
	 { initPartExact (P+i);
           COUNT = searchPartExact (P+i,text,0,n,NULL);
	 }
     /* freePartExact (&P); */
   }


MAIN("partexact",partexactheur);
