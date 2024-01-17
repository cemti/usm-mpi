#include <iostream>
#include <cstdlib>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc != 2)
	{
		if (rank == 0)
			std::cout << "Se cere inca un argument - valoarea rankului 'incep'.";

		MPI_Finalize();
		return 0;
	}

	const int incep = atoi(argv[1]);

	if ((incep & 1) == 0)
	{
		if (rank == 0)
			std::cout << "Se cere ca valoarea rankului 'incep' sa fie impara.";

		MPI_Finalize();
		return 0;
	}

	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (incep >= size || (rank & 1) == 0)
	{
		MPI_Finalize();
		return 0;
	}

	int target = rank - 2, source = rank + 2;

	if (source >= size)
		source = 1;

	if (target < 0)
	{
		target = size - 1;

		if ((target & 1) == 0)
			--target;
	}

	if (rank == incep)
	{
		if (target != rank)
		{
			std::cout << incep << " incepe expedierea mesajelor" << std::endl;
			MPI_Send(NULL, 0, MPI_INT, target, 0, MPI_COMM_WORLD);
			MPI_Recv(NULL, 0, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << rank << " a primit mesaj de la " << source << std::endl;
		}
	}
	else
	{
		MPI_Recv(NULL, 0, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		std::cout << rank << " a primit mesaj de la " << source << std::endl;
		MPI_Send(NULL, 0, MPI_INT, target, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
