#include <iostream>
#include <vector>
#include <ctime>
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int n;

	if (rank == 0)
	{
		std::cout << "n = ";
		std::cin >> n;
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Datatype linie;
	MPI_Type_contiguous(n, MPI_INT, &linie);
	MPI_Type_commit(&linie);

	std::vector<int> v(n);

	if (rank == 0)
	{
		std::vector<int> mtx(size * n);
		std::cout << "Matricea:\n";

		srand(time(NULL));

		for (int i = 0; i < size; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				mtx[i * n + j] = rand() & 65535;
				std::cout << mtx[i * n + j] << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
		MPI_Scatter(&mtx[0], 1, linie, &v[0], 1, linie, 0, MPI_COMM_WORLD);
	}
	else
		MPI_Scatter(NULL, 0, linie, &v[0], 1, linie, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul " << i << ": ";

			for (int j = 0; j < n; ++j)
				std::cout << v[j] << ' ';

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Type_free(&linie);
	MPI_Finalize();
}