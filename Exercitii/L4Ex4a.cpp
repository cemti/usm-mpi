#include <iostream>
#include <cstdlib>
#include <vector>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc != 2)
	{
		if (rank == 0)
			std::cout << "Se necesita inca un argument: nr. de coloane." << std::endl;

		MPI_Finalize();
		return 0;
	}

	int cols = atoi(argv[1]);

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	std::vector<int> line(cols);

	if (rank == 0)
	{
		std::vector<int> mtx(ranks * cols);
		srand(time(NULL));

		std::cout << "Matricea initiala:\n";

		for (int i = 0; i < ranks; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				int& cell = mtx[i * cols + j];
				cell = rand();
				std::cout << cell << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;

		for (int i = 0; i < ranks; ++i)
		{
			if (i == rank)
			{
				for (int j = 0; j < cols; ++j)
					line[j] = mtx[j];

				continue;
			}
			
			for (int j = 0; j < cols; ++j)
				MPI_Sendrecv(&mtx[i * cols + j], 1, MPI_INT, i, 0, NULL, 0, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
	else
	{
		for (int i = 0; i < cols; ++i)
		{
			MPI_Recv(&line[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Send(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}
	}

	if (rank > 0)
		MPI_Recv(NULL, 0, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	std::cout << "Rankul " << rank << " are: ";

	for (int i = 0; i < cols; ++i)
		std::cout << line[i] << ' ';

	std::cout << std::endl;

	if (rank + 1 < ranks)
		MPI_Send(NULL, 0, MPI_INT, rank + 1, 1, MPI_COMM_WORLD);

	MPI_Finalize();
}
