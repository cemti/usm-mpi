#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	if (argc != 2)
	{
		if (rank == 0)
			std::cout << "Se necesita inca un argument: lungimea sirului." << std::endl;

		MPI_Finalize();
		return 0;
	}

	int n = atoi(argv[1]), odds = ranks >> 1, evens = ranks - odds;

	if (odds > n || evens > n)
	{
		if (rank == 0)
			std::cout << "Infezabil: Nr. de procese pare > lungimea sirului < nr. de procese impare." << std::endl;

		MPI_Finalize();
		return 0;
	}

	std::vector<double> masterVec;
	std::vector<int> counts, displs;
	int part;

	if (rank & 1)
	{
		part = n / odds;

		if (((rank - 1) >> 1) + 1 == odds)
			part = n - ((rank - 1) >> 1) * part;
	}
	else
	{
		part = n / evens;

		if ((rank >> 1) + 1 == evens)
			part = n - (rank >> 1) * part;
	}

	double result, value = -INFINITY;
	std::vector<double> vec(part);

	if (rank == 0)
	{
		masterVec.resize(n);
		std::cout << "Tabloul initial:\n";
		srand(time(NULL));

		for (int i = 0; i < n; ++i)
		{
			masterVec[i] = rand();
			std::cout << masterVec[i] << ' ';
		}

		std::cout << std::endl;

		counts.resize(ranks);
		displs.resize(ranks);

		MPI_Gather(&part, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		int displ = 0;

		for (int i = 0; i < ranks; i += 2)
		{
			displs[i] = displ;
			displ += counts[i];
		}

		displ = 0;

		for (int i = 1; i < ranks; i += 2)
		{
			displs[i] = displ;
			displ += counts[i];
		}
	}
	else
		MPI_Gather(&part, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);

	MPI_Scatterv(masterVec.data(), counts.data(), displs.data(), MPI_DOUBLE, vec.data(), vec.size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if ((rank & 1) == 0)
	{
		for (int i = 0; i < vec.size(); ++i)
			if (value < vec[i])
				value = vec[i];
	}

	MPI_Reduce(&value, &result, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	value = INFINITY;

	if (rank & 1)
	{
		for (int i = 0; i < vec.size(); ++i)
			if (value > vec[i])
				value = vec[i];
	}

	MPI_Reduce(&value, &result, 1, MPI_DOUBLE, MPI_MIN, 1, MPI_COMM_WORLD);

	if (rank == 0)
	{
		std::cout << "Max: " << result << std::endl;
		MPI_Send(NULL, 0, MPI_INT, 1, 0, MPI_COMM_WORLD);
	}
	else if (rank == 1)
	{
		MPI_Recv(NULL, 0, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		std::cout << "Min: " << result << std::endl;
	}

	MPI_Finalize();
}
