#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#define BOARD_SIZE 100

typedef struct shared_mem
{
	int board[BOARD_SIZE * BOARD_SIZE];
	int goal;
} shared_mem;
shared_mem *msg;

int main(int argc, char **argv)
{

	int shmid;
	key_t key = 5;
	// if (key == -1)
	// {
	// 	printf("ftok failed");
	// 	return -1;
	// }

	shared_mem *shm;
	msg = (shared_mem *)malloc(sizeof(shared_mem));

	/*
     * Create the segment
    */
	if ((shmid = shmget(key, sizeof(msg), IPC_CREAT|0600)) < 0) 
	{
		perror("shmget");
		exit(1);
	}

	/*
    * Now we attach the segment to our data space.
    */
	if ((shm = shmat(shmid, NULL, 0)) == (char *)-1)
	{
		perror("shmat");
		exit(1);
	}

	msg = shm;

	msg->goal = 64;
}