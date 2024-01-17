#include <iostream>
#include <cstdlib>
#include <vector>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc != 3)
	{
		if (rank == 0)
			std::cout << "Se mai specifica de la terminal, ca argumenzi, inaltimea si latime matricei." << std::endl;

		MPI_Finalize();
		return 0;
	}

	int ranks, sizes[] = { atoi(argv[1]), atoi(argv[2]) };
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	if (ranks > sizes[0])
	{
		if (rank == 0)
			std::cout << "Infezabil: Nr. de procese e mai mult decat numarul de randuri." << std::endl;

		MPI_Finalize();
		return 0;
	}

	std::vector<int> vec, mtx, counts, displs;

	if (rank == 0)
	{
		srand(time(NULL));

		mtx.resize(sizes[0] * sizes[1]);

		std::cout << "Matricea initiala:\n";

		for (int i = 0; i < sizes[0]; ++i)
		{
			for (int j = 0; j < sizes[1]; ++j)
			{
				mtx[i * sizes[1] + j] = rand();
				std::cout << mtx[i * sizes[1] + j] << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
	}

	int part = sizes[0] / ranks;

	if (rank + 1 == ranks)
		part = sizes[0] - rank * part;

	int effectiveSize = sizes[1] * part;
	vec.resize(effectiveSize);

	if (rank == 0)
	{
		counts.resize(ranks);
		MPI_Gather(&effectiveSize, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		displs.resize(ranks);
		int displ = 0;

		for (int i = 0; i < ranks; ++i)
		{
			displs[i] = displ;
			displ += counts[i];
		}
	}
	else
		MPI_Gather(&effectiveSize, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Scatterv(mtx.data(), counts.data(), displs.data(), MPI_INT, vec.data(), vec.size(), MPI_INT, 0, MPI_COMM_WORLD);

	if (rank > 0)
		MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	std::cout << "Rankul " << rank << " a primit:\n";

	for (int i = 0; i < part; ++i)
	{
		for (int j = 0; j < sizes[1]; ++j)
			std::cout << vec[i * sizes[1] + j] << ' ';

		std::cout << '\n';
	}

	std::cout << std::endl;

	if (rank + 1 < ranks)
		MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	
	MPI_Gatherv(vec.data(), vec.size(), MPI_INT, mtx.data(), counts.data(), displs.data(), MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		std::cout << "Matricea finala:\n";

		for (int i = 0; i < sizes[0]; ++i)
		{
			for (int j = 0; j < sizes[1]; ++j)
				std::cout << mtx[i * sizes[1] + j] << ' ';

			std::cout << '\n';
		}

		std::cout << std::endl;
	}

	MPI_Finalize();
}
