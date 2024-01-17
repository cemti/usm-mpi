/*====
In acest program se determina elementele maximale de pe coloanele unei matrici patrate (dimensiunea este egala cu
numarul de procese). Elementele matricei sunt initializate de procesul cu rankul 0.
Se utilizeaza functia MPI_Reduce si operatia MPI_MAX
====*/
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int main(int argc, char* argv[])
{
	int numtask, sendcount, reccount;
	double* A = NULL, *Max_Col = NULL;
	int myrank, root = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtask);

	double *Rows = new double[numtask];
	sendcount = numtask;
	reccount = numtask;

	if (myrank == root)
	{
		printf("\n=====REZULTATUL PROGRAMULUI '%s' \n", argv[0]);
		A = new double[numtask * numtask];
		Max_Col = new double[numtask];

		srand(time(NULL));

		for (int i = 0; i < numtask * numtask; i++)
			A[i] = rand() & 255;

		printf("Tipar datele initiale\n");

		for (int i = 0; i < numtask; i++)
		{
			printf("\n");
			for (int j = 0; j < numtask; j++)
				printf("A[%d,%d]=%5.2f ", i, j, A[i * numtask + j]);
		}

		printf("\n");
	}

	MPI_Win maxWin, rowsWin;
	MPI_Win_create(Max_Col, numtask * sizeof(double), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &maxWin);
	MPI_Win_create(Rows, numtask * sizeof(double), 1, MPI_INFO_NULL, MPI_COMM_WORLD, &rowsWin);

	MPI_Win_fence(0, rowsWin);

	if (myrank == 0)
		for (int i = 0; i < numtask; ++i)
			MPI_Put(A + i * numtask, numtask, MPI_DOUBLE, i, 0, numtask, MPI_DOUBLE, rowsWin);

	MPI_Win_fence(0, rowsWin);

	MPI_Win_fence(0, maxWin);
	MPI_Accumulate(Rows, numtask, MPI_DOUBLE, 0, 0, numtask, MPI_DOUBLE, MPI_MAX, maxWin);
	MPI_Win_fence(0, maxWin);

	if (myrank == root)
	{
		for (int i = 0; i < numtask; i++)
		{
			printf("\n");
			printf("Elementul maximal de pe coloana %d=%5.2f ", i, Max_Col[i]);
		}

		printf("\n");
	}

	MPI_Win_free(&rowsWin);
	MPI_Win_free(&maxWin);
	MPI_Finalize();
	delete[] Rows;
	delete[] A;
	delete[] Max_Col;
}
