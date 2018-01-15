/* This code is from Gonzalo Navarro, 1998.
   It can be freely used for academic teaching/learning and research
   purposes. Usage for direct or indirect commercial advantage is not
   allowed without written permission from the authors.
*/

#include "autom.h"

void printbin(mask x, int pos)
{
	if(pos<=0) {return;}
	
	printbin(x>>1,pos-1);
	printf("%d",x&1);
	
	if(pos==sizeof(x)*8) printf("\n");
}

#define printbin(x) printbin((x),sizeof(x)*8)

        /* searches from text with automaton A. */

int searchFAutom (PartAutom * A, register uchar * text, int from, int to, int *matches)
{
    register mask x;
    register mask D = A->D.d0;
    register mask Din = A->Din;
    register mask m2 = A->msk[2];
    register mask m1 = A->msk[1];
    register mask m3 = m1 | m2;
    register mask G = A->Glast.d0;
    register mask Gact = A->Gany.d0;
    register int p0 = A->p[0];
    register int p3 = A->p[3];
    register mask *T = A->T.d1[0];
    register bool *S = A->S;
    register int n = from;
    int count = 0;

    /* k=m-1 not solved with this automaton */
    if (A->k == A->m - 1)
        return search1match (S, text, from, to, matches);

    if (!(~D & Gact))
        while (S[text[n]])
            n++;

    /* search loop */
    while (true) {
        x = (D >> p0) | T[text[n++]];
        D = ((D << 1) | m1) & ((D << p3) | m3) & (((x + m1) ^ x) >> 1) & Din;
        if (~D & Gact) {        /* active? */
            if (!(D & G)) {     /* found *//* clear the last diagonal */
                D |= m2;
                /* invoke code */
                if (n > 0) {    /* otherwise it was just retraining */
                    if (n <= to) {
                        if (matches != NULL)
                            matches[count] = n;
                        count++;
                    }
                    else
                        break;
                }
                /* may be inactive now (optional optimization) */
                if (!(~D & Gact))
                    while (S[text[n]])
                        n++;
            }
        }
        else
            while (S[text[n]])
                n++;
    }
    A->D.d0 = D;
    return count;
}

int searchAutom (PartAutom * A, register uchar * text, int from, int to, int *matches)
{
    register mask x;
    register mask *T = A->T.d1[0];
    register mask D = A->D.d0;
    register mask Din = A->Din;
    register mask m2 = A->msk[2];
    register mask m1 = A->msk[1];
    register mask m3 = m1 | m2;
    register mask G = A->Glast.d0;
    register int p0 = A->p[0];
    register int p3 = A->p[3];
    register int n = from;
    int count = 0;

    /* k=m-1 not solved with this automaton */
    if (A->k == A->m - 1)
        return search1match (A->S, text, from, to, matches);

    /* search loop */
    while (true) {
        x = (D >> p0) | T[text[n++]];
        D = ((D << 1) | m1) & ((D << p3) | m3) & (((x + m1) ^ x) >> 1) & Din;
        if (!(D & G)) {         /* found *//* clear the last diagonal */
            D |= m2;
            /* fill match */
            if (n > 0) {        /* otherwise it was just retraining */
                if (n <= to) {
                    if (matches != NULL)
                        matches[count] = n;
                    count++;
                }
                else
                    break;
            }
        }
    }
    A->D.d0 = D;
    return count;
}

#define DEBUG 0

//TODO debuguear
int searchVAutom (PartAutom * A, register uchar * text, int from, int to, int *matches)
{
    register mask x;
    register mask *T = A->T.d1[0]; //T['a']=0, T[otra cosa]=219
    register mask D = A->D.d0;
    register mask Din = A->Din;
    register mask m2 = A->msk[2]; // 0..0(1)^(k+1)
    register mask m1 = A->msk[1]; // 0..010..010...
    register mask m3 = m1 | m2; //no se usa
    register mask G = A->Glast.d0; //OK 1<<k
    register int p0 = A->p[0]; //OK = k+2
    register int p3 = A->p[3]; //no se usa
    register int n = from;
    int count = 0;

#if DEBUG
printf("searchVAutom() from %d to %d. diff %d. p0: %d, G: %d\n",from,to,to-from,p0,G);
printf("m1: "); printbin(m1);
printf("m2: "); printbin(m2);
#endif

    /* k=m-1 not solved with this automaton */
    if (A->k == A->m - 1)
        return search1matchV (A->S, text, from, to, matches);

    /* search loop */
    int last = -100;

    while (n < to) {
		#if DEBUG
        
        printf("\nEstoy en n=%d:\n",n);
        
        mask Toriginal = T[text[n++]];
        mask Tnow = ((D >> p0) + m1) & Toriginal;
        
        int shift = (A->m - A->k - 1) * (A->k + 2);
        Tnow |= (Toriginal >> shift) << shift; //Mr Versace
        
        printf("T[%c]\t",text[n-1]); printbin(Toriginal);
		printf("Tnow\t"); printbin(Tnow);
        printf("D\t"); printbin(D);
        //~ printf("mask\t",text[n-1]); printbin(m2<<shift);
        
        #else
        
        mask Tnow = T[text[n++]];
        
        #endif
        
        x = (D >> p0) | Tnow;
        
        D = ((D << 1) | m1) &   //mismatch (suma 1)
            ((D << p3) | m3) & //represents insertions of chars in pattern
            
            (((x + m1) ^ x) >> 1) &     // matches and deletions. TODO work on this!! (suma 1 o mantiene igual)
            
            Din; //máscara que tiene 0s como separadores
        
        #if DEBUG
        printf("x\t"); printbin(x);
        printf("D after\t"); printbin(D);
        #endif
        
        if (!(D & G)) {         /* found */
            D |= m2; /* clear the last diagonal */
            /* fill match */
            if (n > 0) { //capaz >=m
                if (matches != NULL)
                    matches[count] = n;
                if (n != last + 1) //por qué este if ?? TODO
                    count++;

                last = n;
                printf("found %d\n",n-1);
            }
        }
    }
    A->D.d0 = D;
    return count;
}

/*

xxxxxthe hingxxxxx
           ..
           ^matches bcs of deletion in the pattern
            
*/
