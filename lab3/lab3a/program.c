#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "slucajni_prosti_broj.h"

#define MASKA(bitova)			        (-1 + (1<<(bitova)) )
#define UZMIBITOVE(broj,prvi,bitova) 	( ( (broj) >> (64-(prvi)) ) & MASKA(bitova) )
#define N                               10
#define KRAJ_RADA                       -1
#define A                               3 //broj radnih dretvi
#define B                               3 //broj neradnih dretvi
uint64_t MS[N], ulaz = 0, izlaz = 0, BROJAC = 0, velicina_grupe,  
        *BROJ, *ULAZ, kraj;

sem_t KO, prazni, puni;

void stavi_u_MS(uint64_t x)
{
    MS[ulaz] = x;
    ulaz = (ulaz + 1) % N;
    BROJAC++;
    if (BROJAC > N){
        BROJAC--;
        izlaz = (izlaz + 1) % N;
    }
}
uint64_t uzmi_iz_MS()
{    
    uint64_t x = MS[izlaz];
    if (BROJAC > 0){
        izlaz = (izlaz + 1) % N;
        BROJAC--;
    }

    return x;
}



//funkcije iz LAB1

uint64_t count1(uint64_t x) //broji broj pojavljivanja jedinice
{ 
    uint64_t i, sum = 0;
    for (i = 0; i < 4; i++){
        if (x & 1) 
            sum++;
        x >>= 1;
    }
    return sum;
}  

uint64_t zbrckanost(uint64_t x)
{
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

uint64_t generiraj_dobar_broj(uint64_t num, struct gmp_pomocno *p)
{
	uint64_t najbolji_broj = 0, i, broj;
	uint64_t najbolja_zbrckanost = 0, temp_zbrckanost;

	for( i = 0;  i < num; i++) {
		broj = daj_novi_slucajan_prosti_broj(&(*p));
		temp_zbrckanost = zbrckanost(broj);
      
		if (temp_zbrckanost > najbolja_zbrckanost) {
			najbolja_zbrckanost = temp_zbrckanost;
			najbolji_broj = broj;
		}
	}
	return najbolji_broj;
}

uint64_t procjeni_velicinu_grupe()
{
    uint64_t M = 1000, SEK = 10, k = 0, velicina = 1, i, broj;

    struct gmp_pomocno p;
	inicijaliziraj_generator (&p, 0);

    clock_t begin = clock();

    while((double)(clock() - begin)/CLOCKS_PER_SEC < (double)SEK){
        k++;
        for (i = 0; i < M; i++){
            broj = generiraj_dobar_broj(velicina, &p);
            stavi_u_MS(broj);
        }
    }

    obrisi_generator(&p);

    uint64_t br_u_sek = k * M / SEK;
    velicina = br_u_sek * 2 / 5;

    return velicina;
}



//nove funkcije


void udi_u_KO(uint64_t i)
{
    uint64_t j, max = 0;
    ULAZ[i] = 1;
    for (j = 0; j < A; j++)
        if (BROJ[j] > max)
            max = BROJ[j];
    BROJ[i] = max;
    ULAZ[i] = 0;

    for (j = 0; j < A; j++){
        while(ULAZ[j] == 1);

        while(BROJ[j] != 0 && 
            (BROJ[j] < BROJ[i] || (BROJ[j] == BROJ[i] && j < i)));
    }
}

void izadi_iz_KO(uint64_t i) 
{
    BROJ[i] = 0;
}

void *radna_dretva(void *id) 
{
    struct gmp_pomocno p;
    uint64_t *pom = id;
	inicijaliziraj_generator (&p, *pom);

    while (kraj != KRAJ_RADA) {
        uint64_t x = generiraj_dobar_broj(velicina_grupe, &p);
        
        sem_wait(&prazni);
        sem_wait(&KO);

        stavi_u_MS(x);
        printf("stavio %lx\n", x);

        sem_post(&KO);
        sem_post(&puni);
    }

    obrisi_generator (&p);
    return NULL;
}

void *neradna_dretva(void *id) 
{
   // uint64_t *pom = id;
    while(kraj != KRAJ_RADA) {
        sleep(3);

        sem_wait(&puni);
        sem_wait(&KO);

        uint64_t y = uzmi_iz_MS();
        printf("uzeo %lx\n", y);

        sem_post(&KO);
        sem_post(&prazni);
    }

    return NULL;    
}


//main -------------------------------------------------------

int main(int argc, char *argv[])
{
	uint64_t i;

    uint64_t *mem;

    sem_init(&KO, 0, 1);
    sem_init(&puni, 0, 0);
    sem_init(&prazni, 0, N);

    //A radnih, B neradnih dretvi
    mem = malloc(sizeof(int64_t) * 2 * (A+B) + (A+B) * sizeof(pthread_t) );
    ULAZ = mem;
    BROJ = mem + A+B;
    
    pthread_t *t;
    t = (pthread_t*)(BROJ + A+B);
    for (i = 0; i < A+B; i++) {
        BROJ[i] = 0;
        ULAZ[i] = 0;
    }

    velicina_grupe = procjeni_velicinu_grupe();
    
    for (i = 0; i < A; i++){
        BROJ[i] = i;
        if ( pthread_create( &t[i], NULL, radna_dretva, &BROJ[i]) ){
            printf("Ne mogu stvoriti radnu dretvu %ld\n", i);
            exit(1);
        }
    }
    for (i = 0; i < B; i++){
        BROJ[i+A] = i;
        if ( pthread_create( &t[i+A], NULL, neradna_dretva, &BROJ[i+A] ) ){
            printf("Ne mogu stvoriti neradnu dretvu %ld\n", i);
            exit(1);
        }
    }

    sleep(20);
    kraj = KRAJ_RADA;

    for(i = 0; i < A+B; i++){
        pthread_join( t[i], NULL );
    }    

    sem_destroy(&KO);
    sem_destroy(&puni);
    sem_destroy(&prazni);
    free(mem);

	return 0;
}

