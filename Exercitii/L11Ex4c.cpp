#include <iostream>
#include <vector>
#include <cstdlib>
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

		if (n <= 1)
		{
			std::cout << "n <= 1" << std::endl;
			MPI_Abort(MPI_COMM_WORLD, -1);
		}
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::vector<int> v(n - 1);

	if (rank == 0)
	{
		std::vector<int> mtr(n * n);
		srand(time(NULL));

		std::cout << "Matricea:\n";

		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				mtr[i * n + j] = rand() & 65535;
				std::cout << mtr[i * n + j] << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;

		MPI_Datatype type;

		std::vector<int> lens, displs;

		for (int i = 0; i < n - 1; ++i)
		{
			lens.push_back(1);
			displs.push_back(i * n + (n - i - 2));
		}

		MPI_Type_indexed(lens.size(), &lens[0], &displs[0], MPI_INT, &type);
		MPI_Type_commit(&type);

		MPI_Scatter(&mtr[0], 1, type, &v[0], n - 1, MPI_INT, 0, MPI_COMM_SELF);

		MPI_Type_free(&type);
	}

	MPI_Bcast(&v[0], n - 1, MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul " << i << ": ";

			for (int j = 0; j < n - 1; ++j)
				std::cout << v[j] << ' ';

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}