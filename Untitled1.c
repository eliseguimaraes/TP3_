#include <stdlib.h>
#include <stdio.h>
#include <math.h>
int commonFactor (int a, int b) {
    if (a>b) {
        if (a%b==0) return b;
        else return (commonFactor(a%b,b));
    }
    else {
        if(b%a==0) return a;
        else return (commonFactor(b%a,a));
    }
}

int commonFactor3 (int a, int b, int c) {
    return (commonFactor(commonFactor(a,b),c));
}


int main () {
    int A,B,C,x,y,z,comum,r;
    long long int aux;
    int bmin, bmax, pmin,pmax;
    float auxz;
    bmin = 2;
    bmax = 300;
    pmin = 3;
    pmax = 9;
    for (A=bmin; A<=bmax; A++) {
        for (x=pmin; x<=pmax; x++) {
            for (B=bmin; B<=bmax; B++) {
                for (y=pmin; y<=pmax; y++) {
                    aux = pow(A,x) + pow(B,y);
                    for (z = pmin; z<=pmax; z++) {
                        C = powf(aux, ((float)1/(float)z));
                        r = pow(C,z);
                            if (r==aux && z>2) {
                                comum = commonFactor3(A,B,C);
                                if (comum!=1) {
                                    printf("\n%d^%d + %d^%d = %d^%d, com fator primo %d\n",A,x,B,y,C,z,comum);
                                }
                                else {
                                    printf("\n%d^%d + %d^%d = %d^%d, E NAO HA FATOR PRIMO!",A,x,B,y,C,z,comum);
                                }
                            }
                        }
                    }
                }
            }
        }
    return 0;
}
