#include "queue.h"

int main()
{
    struct storage farszMag, ciastoMag, pierogiMag;
    pid_t wFarszM, wFarszG, wCiasto, wPierogi, dyspozytornia;

    int i;

    /// semaphores initialization
    farszMag.semid = semget(1000, 3, IPC_CREAT|0600);
    ciastoMag.semid = semget(1001, 3, IPC_CREAT|0600);
    pierogiMag.semid = semget(1002, 3, IPC_CREAT|0600);

    /// set initial values
    semctl(farszMag.semid, MUTEX, SETVAL, (int)1);
    semctl(farszMag.semid, FREE_SPACE, SETVAL, (int)MAX_QUEUE);
    semctl(farszMag.semid, IN_QUEUE, SETVAL, (int)0);

    semctl(ciastoMag.semid, MUTEX, SETVAL, (int)1);
    semctl(ciastoMag.semid, FREE_SPACE, SETVAL, (int)MAX_QUEUE);
    semctl(ciastoMag.semid, IN_QUEUE, SETVAL, (int)0);

    semctl(pierogiMag.semid, MUTEX, SETVAL, (int)1);
    semctl(pierogiMag.semid, FREE_SPACE, SETVAL, (int)MAX_QUEUE);
    semctl(pierogiMag.semid, IN_QUEUE, SETVAL, (int)0);

    /// shared memory allocation
    farszMag.shmid = shmget(1000, (MAX_QUEUE + 2)*sizeof(int), IPC_CREAT|0600); //buffer + liczba przedm w mag
    if(farszMag.shmid == -1)
    {
        printf( RED "Blad podczas alokacji pamieci dla magazynu farszu! Blad: %s\n" RED, strerror(errno));
        exit(1);
    }
    farszMag.buffer = (int *)shmat(farszMag.shmid, NULL, 0);    // getting address of alocated memory

    ciastoMag.shmid = shmget(1001, (MAX_QUEUE + 2)*sizeof(int), IPC_CREAT|0600);

    if(ciastoMag.shmid == -1)
    {
        printf( RED "Blad podczas alokacji pamieci dla magazynu ciasta! Blad: %s\n" RED, strerror(errno));
        exit(1);
    }

    ciastoMag.buffer = (int *)shmat(ciastoMag.shmid, NULL, 0);    // getting address of alocated memory

    pierogiMag.shmid = shmget(1002, (MAX_QUEUE + 2)*sizeof(int), IPC_CREAT|0600);
    if(pierogiMag.shmid == -1)
    {
        printf( RED "Blad podczas alokacji pamieci dla magazynu pierogow! Blad: %s\n" RED, strerror(errno));
        exit(1);
    }
    pierogiMag.buffer = (int *)shmat(pierogiMag.shmid, NULL, 0);    // getting address of alocated memory

    // default values in buffer
    for(i = 0; i <= (MAX_QUEUE + 1); ++i)
    {
        farszMag.buffer[i] = 0;
        ciastoMag.buffer[i] = 0;
        pierogiMag.buffer[i] = 0;
    }

    /// TEST

    // show default semaphores values
    showSemaphoresStatus(&farszMag);
    showSemaphoresStatus(&ciastoMag);
    showSemaphoresStatus(&pierogiMag);

    getchar();

    /// create producers processes

    // wytwórnia farszu mięsnego
    if(!(wFarszM = fork())) // fork returns zero in child process
    {
        producer(&farszMag, 200000); // 5/s
        return 0;
    }

    // wytwórnia farszu grzybowego
    if(!(wFarszG = fork())) // fork returns zero in child process
    {
        priorProducer(&farszMag, 2000000); // 0.5/s
        return 0;
    }

    // wytwórnia ciasta
    if(!(wCiasto = fork())) // fork returns zero in child process
    {
        producer(&ciastoMag, 143000); // okolo 7/s
        return 0;
    }

    /// create consumers processes

    // wytwornia pierogow
    if(!(wPierogi = fork())) // fork returns zero in child process
    {
        prodCons(&farszMag, &ciastoMag, &pierogiMag, 250000); // 4/s
        return 0;
    }

    // dyspozytornia
    if(!(dyspozytornia = fork())) // fork returns zero in child process
    {
        consumer(&pierogiMag, 333000); // 3/s
        return 0;
    }

    /// simulation time
    sleep(SIMULATION_TIME);

    /// kill all child processes
    kill(wFarszM, SIGTERM);
    kill(wFarszG, SIGTERM);
    kill(wCiasto, SIGTERM);
    kill(wPierogi, SIGTERM);
    kill(dyspozytornia, SIGTERM);

    /// shared memory deallocation
    shmctl(farszMag.shmid, IPC_RMID, (struct shmid_ds *)0);
    shmctl(ciastoMag.shmid, IPC_RMID, (struct shmid_ds *)0);
    shmctl(pierogiMag.shmid, IPC_RMID, (struct shmid_ds *)0);

    /// semaphores deallocation
    semctl(farszMag.semid, 0, IPC_RMID, (struct semid_ds *)0);
    semctl(ciastoMag.semid, 0, IPC_RMID, (struct semid_ds *)0);
    semctl(pierogiMag.semid, 0, IPC_RMID, (struct semid_ds *)0);

    return 0;
}
