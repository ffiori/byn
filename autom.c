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




typedef unsigned long word;
#define W (8 * (int)sizeof(word))

static int B; /* number of bits per state */
static word lim, ovmask, MASK, T[256];

/* ceil(log2(x)) */
int clog2(size_t x)
{
    int i;
    for (i = 0; ((size_t)1 << i) < x; i++);
    return i;
}

void prep(unsigned char *pat, size_t m, size_t k)
{
    size_t i;

    B = clog2(k+1) + 1;
    if (m*B > W) {
        fprintf(stderr, "need m*B=%lu > %d=W bits\n", (unsigned long)(m*B), W);
        exit(42);
    }

    lim = (word)k << (m-1)*B;
    ovmask = 0;
    for (i = 1; i <= m; i++)
        ovmask = (ovmask << B) | ((word)1 << (B-1));
    lim += (word)1 << (m-1)*B;
    for (i=0; i < 256; i++) T[i] = ovmask >> (B-1);
    for (i = 1; *pat != '\0'; i <<= B) {
        T[*pat] &= ~i;
        pat++;
    }

    MASK = (m*B == W) ? ~(word)0 : i-1;
}

size_t exec(unsigned char *text, size_t n, size_t k)
{
    word state, overflow;
    size_t i, occ = 0;

    state = MASK & ~ovmask;
    overflow = ovmask;

    occ = 0;
    for (i = 0; i < n; i++) {
        state = ((state << B) + T[text[i]]) & MASK;
        overflow = ((overflow << B) | (state & ovmask)) & MASK;
        state &= ~ovmask;
        if ((state + overflow) < lim) {
            /* match at T[i-m+1..i] */
            occ++;
        }
    }

    return occ;
}






#define DEBUG 1

int searchVAutom (PartAutom * A, register uchar * text, int from, int to, int *matches)
{
    register mask x;
    //~ register mask *T = A->T.d1[0]; //T['a']=0, T[otra cosa]=219
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
	int k = p0-2; //WTF
	
	
#if DEBUG
printf("searchVAutom() from %d to %d. diff %d. p0: %d, G: %d\n",from,to,to-from,p0,G);
printf("m1: "); printbin(m1);
printf("m2: "); printbin(m2);
#endif
	
	prep("aaaa",4,k);
	
	printbin(MASK);
	printbin(ovmask);
	printbin(T['a']);
	printbin(T['b']);
	
	count = exec(text+from, to-from, k); 
	printf("count: %d\n",count);
	return count;
}




#if 0

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
        mask Tnow = ((D >> p0) + m1) & Toriginal; //conservo sólo el 1 a la izq del primer uno de cada diagonal (que sería el mismatch)
        
        int shift = (A->m - A->k - 1) * (A->k + 2);
        Tnow |= (Toriginal >> shift) << shift; //primera diagonal
        
        printf("T[%c]\t",text[n-1]); printbin(Toriginal);
		printf("Tnow\t"); printbin(Tnow);
        printf("D\t"); printbin(D);
        //~ printf("mask\t",text[n-1]); printbin(m2<<shift);
        
        #else
        
        mask Tnow = T[text[n++]];
        
        #endif
        
        x = (D >> p0) | Tnow; // D >> (k+2), o sea que desplaza las diagonales a la derecha.
        
        D = 
			((D << 1) | m1) &   //mismatch (suma 1, necesario solamente para la primer diagonal)
			
            ((D << p3) | m3) & //represents insertions of chars in pattern
            
            (((x + m1) ^ x) >> 1) &     // matches and deletions. TODO work on this!! (suma 1 o mantiene igual, excepto en la primer diagonal)
            
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


#endif

/*

xxxxxthe hingxxxxx
           ..
           ^matches bcs of deletion in the pattern
            
*/
