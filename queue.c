#include "queue.h"

/// producer-consumer functions

void producer(struct storage *s, int microSecPeriod)
{
    int i;

    while (1)
    {
        // production time
        // suspend executing thread for requested number of microseconds
        if(microSecPeriod != 0)
        {
            usleep(microSecPeriod);
        }

        if(s->buffer[MAX_QUEUE] == (MAX_QUEUE))
        {
            printf( YELLOW "Magazyn %d pelny!\n" YELLOW, s->shmid);
        }

        // semaphore operations
        down(FREE_SPACE, s->semid);
        down(MUTEX, s->semid);
        up(IN_QUEUE, s->semid);

        /// insert to buffer

        // find free place
        for(i = 0; s->buffer[i] != 0; ++i);

        // put value to storage
        s->buffer[i] = 1;

        // increase number of products in storage
        ++(s->buffer[MAX_QUEUE]);

        /// TEST
        printf(ANSI_COLOR_CYAN "\nWstawiono do %d na indeks %d\n" ANSI_COLOR_CYAN, s->shmid, i);
        printf(ANSI_COLOR_CYAN "Aktualna ilosc el w magazynie: %d\n\n" ANSI_COLOR_CYAN, s->buffer[MAX_QUEUE]);

        if(microSecPeriod == 0)
        {
            printf(RED "Pojedynczy\n\n" RED);
        }

        showSemaphoresStatus(s);

        // leave shared memory
        up(MUTEX, s->semid);

        if(microSecPeriod == 0)
        {
            return;
        }
    }
}

void priorProducer(struct storage *s, int microSecPeriod)
{
    int i;

    while (1)
    {
        // wait for user request for prior product, x and enter - exit prior
        if((microSecPeriod != 0) && (getchar() == 'x')) return;

        // suspend executing thread for requested number of microseconds
        if(microSecPeriod != 0)
        {
            srand (time(NULL));
            usleep(rand()%4000000); 
        }

        if(s->buffer[MAX_QUEUE] == (MAX_QUEUE))
        {
           // printf( RED "Magazyn %d pelny!\n" RED, s->shmid);
        }

        // semaphore operations
        down(MUTEX, s->semid);
        down(FREE_SPACE, s->semid);
        up(IN_QUEUE, s->semid);

        /// insert to buffer
        i = insertPrior(s->buffer);

        // increase number of products in storage
        ++(s->buffer[MAX_QUEUE]);
        ++(s->buffer[MAX_QUEUE+1]);

        /// TEST
        printf(YELLOW "\nWstawiono priorytetowy do %d na indeks %d\n" YELLOW, s->shmid, i);
        printf(YELLOW "Aktualna ilosc el w magazynie: %d\n\n" YELLOW, s->buffer[MAX_QUEUE]);

        if(microSecPeriod == 0)
        {
            printf(RED "Pojedynczy\n\n" RED);
        }

        showSemaphoresStatus(s);

        // leave shared memory
        up(MUTEX, s->semid);

        if(microSecPeriod == 0)
        {
            break;
        }
    }
}

int consumer(struct storage *s, int microSecPeriod)
{
    int prior;

    while (1)
    {
        if(s->buffer[MAX_QUEUE] == 0)
        {
            //printf( RED "Magazyn %d pusty!\n" RED, s->shmid);
        }

        // semaphore operations
        down(IN_QUEUE, s->semid);
        down(MUTEX, s->semid);
        up(FREE_SPACE, s->semid);

        /// remove from buffer
        prior = getFromStorage(s->buffer);

        // decrease number of products in storage
        --(s->buffer[MAX_QUEUE]);
        if(prior) --(s->buffer[MAX_QUEUE+1]);

        /// TEST
        if(prior) printf(YELLOW "\nPRODUKT PRIORYTETOWY" YELLOW);
        printf(MAGENTA "\nZabrano z %d\n" MAGENTA, s->shmid);
        printf(MAGENTA "Aktualna ilosc el w magazynie: %d\n\n" MAGENTA, s->buffer[MAX_QUEUE]);

        showSemaphoresStatus(s);

         // leave shared memory
        up(MUTEX, s->semid);

         // processing time
        // suspend executing thread for requested number of microseconds
        if(microSecPeriod != 0)
        {
            usleep(microSecPeriod);
        }

        printf(GREEN "\nKonsument przetworzyl produkt\n\n" GREEN);

        if(microSecPeriod == 0)
        {
            break;
        }
    }

    return prior;
}

void prodCons(struct storage *sFarsz, struct storage *sCiasto, struct storage *sPierogi, int microSecPeriod)
{
    int prior;

    while (1)
    {
        /// get farsz and ciasto
        if(consumer(sFarsz, 0) == 1)
        {
            prior = 1;
        }
        else prior = 0;

        consumer(sCiasto, 0);

        /// production time
        usleep(microSecPeriod);

        /// insert pieroga
        if(prior == 1)
        {
            priorProducer(sPierogi, 0);
        }
        else producer(sPierogi, 0);
    }
}

/// semaphore operations

void up(int num, int id)
{
    struct sembuf buf;
    buf.sem_num = num;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semop( id, &buf, 1);
}

void down(int num, int id)
{
    struct sembuf buf;
    buf.sem_num = num;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop( id, &buf, 1);

}

void showSemaphoresStatus(struct storage *s)
{
    int t1, t2, t3;

    t1 = semctl(s->semid, MUTEX, GETVAL, 0);
    t2 = semctl(s->semid, FREE_SPACE, GETVAL, 0);
    t3 = semctl(s->semid, IN_QUEUE, GETVAL, 0);
    printf("Semafory magazynu o id_shm %d: MUTEX = %d, FREE_SPACE = %d, IN_QUEUE = %d\n", s->shmid, t1, t2, t3);
}

/// buffer modifications

int getFromStorage(int *buffer)
{
    int i, prior = 0;

    if (buffer[0] == 2) prior = 1;

    for(i = 0; (buffer[i+1] != 0) && (i < (MAX_QUEUE - 1)); ++i)
    {
        buffer[i] = buffer[i+1];
    }

    if(i < MAX_QUEUE) buffer[i] = 0;

    if(prior) return 1; // prior product

    return 0; // normal product
}

int insertPrior(int *buffer)
{
    int firstNotPrior, i;

    if(buffer[0] == 0)
    {
        buffer[0] = 2;
        return -1;
    }

    // leave prior products as they lay (FIFO for prior)
    for(firstNotPrior=0; buffer[firstNotPrior] == 2; ++firstNotPrior);

    // go to the first place after last product in queue
    for(i=0 ; (buffer[i] != 0) && (i < MAX_QUEUE); ++i);

    // change placement of products
    for( ; i > firstNotPrior; --i)
    {
        buffer[i] = buffer[i-1];
    }

    // insert prior
    buffer[firstNotPrior] = 2;

    return firstNotPrior;
}
