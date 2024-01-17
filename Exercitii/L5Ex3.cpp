#include <iostream>
#include <cstdlib>
#include <vector>
#include <mpi.h>

struct IntInt
{
	int value, index;
};

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	if (argc != 3)
	{
		if (rank == 0)
			std::cout << "Se necesita inca 2 argumenzi: lungimea si latimea matricei." << std::endl;

		MPI_Finalize();
		return 0;
	}

	int sizes[] = { atoi(argv[1]), atoi(argv[2]) };

	if (ranks > sizes[0])
	{
		if (rank == 0)
			std::cout << "Infezabil: Nr. de procese este mai mare decat nr. de linii." << std::endl;

		MPI_Finalize();
		return 0;
	}

	std::vector<IntInt> mtx;
	std::vector<int> counts, displs;

	if (rank == 0)
	{
		std::cout << "Matricea initiala:\n";
		mtx.resize(sizes[0] * sizes[1]);
		srand(time(NULL));

		for (int i = 0; i < sizes[0]; ++i)
		{
			for (int j = 0; j < sizes[1]; ++j)
			{
				IntInt& cell = mtx[i * sizes[1] + j];

				cell.value = rand();
				cell.index = i;

				std::cout << cell.value << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
	}

	int part = sizes[0] / ranks;

	if (rank + 1 == ranks)
		part = sizes[0] - part * rank;

	int effectiveSize = part * sizes[1];

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

	std::vector<IntInt> line(effectiveSize);
	MPI_Scatterv(mtx.data(), counts.data(), displs.data(), MPI_2INT, line.data(), line.size(), MPI_2INT, 0, MPI_COMM_WORLD);

	for (int j = 0; j < sizes[1]; ++j)
	{
		IntInt& mx = line[j];

		for (int i = 1; i < part; ++i)
			if (line[i * sizes[1] + j].value > mx.value)
				mx = line[i * sizes[1] + j];
	}

	if (rank == 0)
	{
		std::vector<IntInt> maxs(sizes[1]);
		MPI_Reduce(line.data(), maxs.data(), sizes[1], MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

		std::cout << "Elementele maximale de pe coloane (valoare, rank/linia):\n";

		for (int i = 0; i < sizes[1]; ++i)
			std::cout << '(' << maxs[i].value << ", " << maxs[i].index << "), ";

		std::cout << std::endl;
	}
	else
		MPI_Reduce(line.data(), NULL, sizes[1], MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);
	
	MPI_Finalize();
}
