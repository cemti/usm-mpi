#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <climits>
#include <ctime>
#include <cmath>
#include <mpi.h>

void parent(const char* command, int n, int l)
{
	MPI_Comm comm;

	{
		char str[11], *argv[] = { str, NULL };
		strcpy(str, std::to_string(n).c_str());
		
		MPI_Comm icomm;
		MPI_Comm_spawn(command, argv, l, MPI_INFO_NULL, 0, MPI_COMM_SELF, &icomm, MPI_ERRCODES_IGNORE);
		MPI_Intercomm_merge(icomm, 0, &comm);
		MPI_Comm_free(&icomm);
	}

	srand(time(NULL));

	std::vector<std::vector<int>> x(l, std::vector<int>(n));

	for (int i = 0; i < l; ++i)
	{
		std::cout << "Vectorul " << i << ": ";
		
		for (int j = 0; j < n; ++j)
		{
			x[i][j] = rand() & 65535;
			std::cout << x[i][j] << ' ';
		}

		MPI_Send(&x[i][0], n, MPI_INT, i + 1, 0, comm);
		std::cout << '\n';
	}

	std::cout << std::endl;

	float norm = INFINITY;
	MPI_Reduce(MPI_IN_PLACE, &norm, 1, MPI_FLOAT, MPI_MIN, 0, comm);
	std::cout << "Valoarea minimala a normei: " << norm << std::endl;
	MPI_Comm_free(&comm);
}

void child(int n)
{
	MPI_Comm comm;

	{
		MPI_Comm icomm;
		MPI_Comm_get_parent(&icomm);
		MPI_Intercomm_merge(icomm, 1, &comm);
		MPI_Comm_free(&icomm);
	}

	std::vector<int> x(n);
	MPI_Recv(&x[0], n, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);

	float norm = 0;

	for (int i = 0; i < n; ++i)
		norm += x[i] * x[i];

	norm = sqrt(norm);

	int size, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	for (int i = 0; i < size; ++i)
	{
		if (i == rank)
		{
			std::cout << "Rankul copil " << i << " are: ";

			for (int j = 0; j < n; ++j)
				std::cout << x[j] << ' ';

			std::cout << "\n Cu norma vectorului = " << norm << std::endl;
		}

		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Reduce(&norm, NULL, 1, MPI_FLOAT, MPI_MIN, 0, comm);
	MPI_Comm_free(&comm);
}

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	if (argc <= 1)
	{
		int n, l;

		std::cout << "n si l:" << std::endl;
		std::cin >> n >> l;

		parent(argv[0], n, l);
	}
	else
	{
		child(atoi(argv[1]));
	}

	MPI_Finalize();
}