/*
 * Jednostavni primjer za generiranje prostih brojeva korištenjem
   "The GNU Multiple Precision Arithmetic Library" (GMP)
 */

#include <stdio.h>
#include <time.h>

#include "slucajni_prosti_broj.h"

#define MASKA(bitova)			        (-1 + (1<<(bitova)) )
#define UZMIBITOVE(broj,prvi,bitova) 	( ( (broj) >> (64-(prvi)) ) & MASKA(bitova) )
#define N                               10
struct gmp_pomocno p;

uint64_t MS[N], end_poz = 0;


void stavi_u_MS(uint64_t x){
    if (end_poz == N){
        uint64_t  i;
        for (i = 0; i < N - 1; i++)
            MS[i] = MS[i + 1];
        MS[N - 1] = x;
        return;
    }
    MS[end_poz] = x;
    end_poz++;
}
uint64_t uzmi_iz_MS(){
    
    end_poz--;

    return MS[0];
}



uint64_t count1(uint64_t x){ //broji broj pojavljivanja jedinice
    uint64_t i, sum = 0;
    for (i = 0; i < 4; i++){
        if (x & 1) 
            sum++;
        x >>= 1;
    }
    return sum;
}  
/*
uint64_t zbrckanost(uint64_t x){

    uint64_t z = 0, i, j,  b1 = 0, pn;

    for (i = 0; i < 64 -6; i++){
        //podniz bitova od x: od x[i] do x[i+5]
        pn = UZMIBITOVE(x, i+6, 6);
        for(j = 0; j < 6; j++)
            if( (1<<j) & pn )
                b1++;
        
        if (b1 > 4)
            z += b1 - 4;
        else if (b1 < 2)
            z += 2 - b1;
    }
    return b1;
}*/

uint64_t zbrckanost(uint64_t x){

    uint64_t i, b1 = 0, pn, gn;

    for (i = 0; i < 64 - 8; i++){
        //podniz bitova od x: od x[i] do x[i+3], tj x[i+4] do x[i+7]
        pn = UZMIBITOVE(x, i + 4, 4);
        gn = UZMIBITOVE(x, i + 8, 4);
        if ( count1(pn) != count1(gn) )
            b1++;
    }
    return b1;
}

uint64_t generiraj_dobar_broj(uint64_t num)
{
	uint64_t najbolji_broj = 0, i, broj;
	uint64_t najbolja_zbrckanost = 0, temp_zbrckanost;

	for( i = 0;  i < num; i++) {
		broj = daj_novi_slucajan_prosti_broj(&p);
		temp_zbrckanost = zbrckanost(broj);
      
		if (temp_zbrckanost > najbolja_zbrckanost) {
			najbolja_zbrckanost = temp_zbrckanost;
			najbolji_broj = broj;
		}
	}
	return najbolji_broj;
}

uint64_t procjeni_velicinu_grupe(){

    uint64_t M = 1000, SEK = 10, k = 0, velicina_grupe = 1, i, broj;

    clock_t begin = clock();

    while((double)(clock() - begin)/CLOCKS_PER_SEC < (double)SEK){
        k++;
        for (i = 0; i < M; i++){
            broj = generiraj_dobar_broj(velicina_grupe);
            stavi_u_MS(broj);
        }
    }
    uint64_t br_u_sek = k * M / SEK;
    velicina_grupe = br_u_sek * 2 / 5;

    return velicina_grupe;
}


int main(int argc, char *argv[])
{
	uint64_t broj, velicina_grupe, broj_ispisa = 0;
	inicijaliziraj_generator (&p, 0);

    velicina_grupe = procjeni_velicinu_grupe();
    
    clock_t t = clock()/CLOCKS_PER_SEC;
    while (broj_ispisa < 10){

        broj = generiraj_dobar_broj(velicina_grupe);

        stavi_u_MS(broj);

        if(clock()/CLOCKS_PER_SEC != t){
            broj = uzmi_iz_MS();
            printf("%lx\n", broj);
            broj_ispisa++;
            t = clock()/CLOCKS_PER_SEC;
        }
    }


	obrisi_generator (&p);

	return 0;
}

/*
  prevođenje:
  - ručno: gcc program.c slucajni_prosti_broj.c -lgmp -lm -o program
  - preko Makefile-a: make
  pokretanje:
  - ./program
  - ili: make pokreni
  nepotrebne datoteke (.o, .d, program) NE stavljati u repozitorij
  - obrisati ih ručno ili s make obrisi
*/