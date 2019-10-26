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
	
}