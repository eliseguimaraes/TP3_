#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct set {
    int A, B, C, x, y, z, comum;
};
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

int searchResult (int bmin, int bmax, int pmin, int pmax, struct set *result) {
    int A,B,C,x,y,z,comum,r;
    long long int aux;
    int i = 0;
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
                                    //printf("\n%d^%d + %d^%d = %d^%d, com fator primo %d\n",A,x,B,y,C,z,comum);
                                }
                                else {
                                    result[i].A = A;
                                    result[i].B = B;
                                    result[i].C = C;
                                    result[i].x = x;
                                    result[i].y = y;
                                    result[i].z = z;
                                    result[i].comum = comum;
                                    i++;
                                }
                            }
                        }
                    }
                }
            }
        }
        return i; //retorna o numero de resultados encontrados
}

int main () {
    int bmin, bmax, pmin,pmax, num,i;
    struct set result[100];
    bmin = 600;
    bmax = 800;
    pmin = 3;
    pmax = 9;
    num = searchResult(bmin, bmax, pmin, pmax, result);
    for (i=0; i<num; i++) {
        printf("\n%d^%d + %d^%d = %d^%d, E NAO HA FATOR PRIMO!",result[i].A,result[i].x,result[i].B,result[i].y,result[i].C,result[i].z,result[i].comum);
    }    return 0;
}
