#include <iostream>
#include <cstdlib>
#include <vector>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int sizes[2];

	if (rank == 0)
	{
		std::cout << "Introduceti inaltimea si latimea: ";
		std::cin >> sizes[0] >> sizes[1];
	}

	MPI_Bcast(sizes, 2, MPI_INT, 0, MPI_COMM_WORLD);

	int ranks;
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

	MPI_Barrier(MPI_COMM_WORLD);

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

	for (int i = 0; i < ranks; ++i)
	{
		if (rank == i)
		{
			std::cout << "Rankul " << rank << " a primit:\n";

			for (int i = 0; i < part; ++i)
			{
				for (int j = 0; j < sizes[1]; ++j)
					std::cout << vec[i * sizes[1] + j] << ' ';

				std::cout << '\n';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}
