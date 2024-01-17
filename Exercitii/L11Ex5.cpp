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
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::vector<int> mtx((n * (n + 1)) >> 1);

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

		for (int i = 0; i < n; ++i)
		{
			lens.push_back(i + 1);
			displs.push_back(i * n);
		}

		MPI_Type_indexed(lens.size(), &lens[0], &displs[0], MPI_INT, &type);
		MPI_Type_commit(&type);

		MPI_Scatter(&mtr[0], 1, type, &mtx[0], mtx.size(), MPI_INT, 0, MPI_COMM_SELF);

		MPI_Type_free(&type);
	}

	MPI_Bcast(&mtx[0], mtx.size(), MPI_INT, 0, MPI_COMM_WORLD);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul " << i << ":\n";
			int c = 0;

			for (int j = 0; j < n; ++j)
			{
				for (int k = 0; k <= j; ++k)
					std::cout << mtx[c++] << ' ';

				std::cout << '\n';
			}

			std::cout << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();
}