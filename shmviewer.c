#include "queue.h"

int main()
{
  int farszId, ciastoId, pierogiId;
  int *farsz, *ciasto, *pierogi;
  int temp = 0, tempPrior = 0, zrobione = 0, zrobionePrior = 0;
  
  if((farszId = shmget(1000, (MAX_QUEUE + 2)*sizeof(int), 0600)) < 0)
  {
		printf(RED "Blad pobrania id pamieci magazynu farszu\n" RED);
		exit(1);
	}
	
  farsz = (int *)shmat(farszId, NULL, 0);

  if((ciastoId = shmget(1001, (MAX_QUEUE + 2)*sizeof(int), 0600)) < 0)
  {
		printf(RED "Blad pobrania id pamieci magazynu ciasta\n" RED);
		exit(1);
	}
  
  ciasto = (int *)shmat(ciastoId, NULL, 0);

  if((pierogiId = shmget(1002, (MAX_QUEUE + 2)*sizeof(int), 0600)) < 0)
  {
		printf(RED "Blad pobrania id pamieci magazynu pierogow\n" RED);
		exit(1);
	}

  pierogi = (int *)shmat(pierogiId, NULL, 0);
  
  while(shmget(1000, (MAX_QUEUE + 1)*sizeof(int), 0600) >= 0)
  {
    if((temp - pierogi[MAX_QUEUE]) > 0) ++zrobione;
    if((tempPrior - pierogi[MAX_QUEUE+1]) > 0) ++zrobionePrior;
    temp = pierogi[MAX_QUEUE];
    tempPrior = pierogi[MAX_QUEUE+1];

    printf("Farsz-> %d %d\tCiasto-> %d\tPierogi-> %d %d\t\t Zrobione-> %d %d\n", 
              farsz[MAX_QUEUE], farsz[MAX_QUEUE+1], ciasto[MAX_QUEUE], pierogi[MAX_QUEUE], pierogi[MAX_QUEUE+1], zrobione, zrobionePrior);
    usleep(100000);
  }
  
  shmctl(farszId, IPC_RMID, (struct shmid_ds *)0);
  shmctl(ciastoId, IPC_RMID, (struct shmid_ds *)0);
  shmctl(pierogiId, IPC_RMID, (struct shmid_ds *)0);

  return 0;
}
