#include <iostream>
#include <vector>
#include <map>
#include <mpi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (argc < 7)
	{
		if (rank == 0)
			std::cout << "Se cere sase parametri/dimensiuni (grila, matrice, submatrice)." << std::endl;

		MPI_Finalize();
		return 0;
	}

	int gridDims[] = { atoi(argv[1]), atoi(argv[2]) },
		sizes[] = { atoi(argv[3]), atoi(argv[4]) },
		blocks[] = { atoi(argv[5]), atoi(argv[6]) };
		
	int nRanks;
	MPI_Comm_size(MPI_COMM_WORLD, &nRanks);
	
	if (nRanks < gridDims[0] * gridDims[1])
	{
		if (rank == 0)
			std::cout << "Insuficient nr. de procese pentru grila." << std::endl;

		MPI_Finalize();
		return 0;
	}

	MPI_Comm truncComm;
	
	{
		std::vector<int> gridVec(gridDims[0] * gridDims[1]);

		for (int i = 0; i < gridVec.size(); ++i)
			gridVec[i] = i;

		MPI_Group group1, group2;
		MPI_Comm_group(MPI_COMM_WORLD, &group1);
		MPI_Group_incl(group1, int(gridVec.size()), gridVec.data(), &group2);
		MPI_Comm_create(MPI_COMM_WORLD, group2, &truncComm);

		MPI_Group_free(&group2);
		MPI_Group_free(&group1);
	}

	if (truncComm == MPI_COMM_NULL)
	{
		MPI_Finalize();
		return 0;
	}

	int gridRank;
	MPI_Comm_rank(truncComm, &gridRank);
	
	MPI_Comm_size(truncComm, &nRanks);

	MPI_Comm gridComm;
	
	{
		int periods[2] = { 1, 1 };
		MPI_Cart_create(truncComm, 2, gridDims, periods, false, &gridComm);
	}

	std::map<std::pair<int, int>, int> map;

	if (gridRank == 0)
	{
		srand(time(NULL));
		std::cout << "Matricea initiala:\n";

		std::vector<std::vector<std::pair<int, int>>> locations(nRanks);
		std::vector<std::vector<int>> values(nRanks);
		
		int coords[2] = {}, rowIt = 0;

		for (int i = 0; i < sizes[0]; ++i)
		{
			int colIt = 0;
			coords[1] = 0;
			
			for (int j = 0; j < sizes[1]; ++j)
			{
				int value = rand() & 255;
				std::cout << value << ' ';
				
				int target;
				MPI_Cart_rank(gridComm, coords, &target);

				locations[target].push_back({ i, j });
				values[target].push_back(value);

				if (++colIt >= blocks[1])
				{
					++coords[1];
					colIt = 0;
				}
			}

			std::cout << '\n';

			if (++rowIt >= blocks[0])
			{
				++coords[0];
				rowIt = 0;
			}
		}

		std::cout << std::endl;

		for (int i = 0; i < nRanks; ++i)
		{
			int count = int(locations[i].size());
			
			if (i == 0)
			{
				for (int i = 0; i < count; ++i)
					map[locations[0][i]] = values[0][i];

				continue;
			}
			
			MPI_Send(locations[i].data(), count, MPI_2INT, i, 0, gridComm);
			MPI_Send(values[i].data(), count, MPI_INT, i, 0, gridComm);
		}
	}
	else
	{
		MPI_Status status;
		MPI_Probe(0, 0, gridComm, &status);

		int count;
		MPI_Get_count(&status, MPI_2INT, &count);

		std::vector<std::pair<int, int>> locations(count);
		MPI_Recv(locations.data(), count, MPI_2INT, 0, 0, gridComm, MPI_STATUS_IGNORE);

		std::vector<int> values(count);
		MPI_Recv(values.data(), count, MPI_INT, 0, 0, gridComm, MPI_STATUS_IGNORE);

		for (int i = 0; i < count; ++i)
			map[locations[i]] = values[i];
	}

	for (int i = 0; i < nRanks; ++i)
	{
		if (rank == i)
		{
			std::cout << "Rankul " << i << ":\n";
			
			int nl = -1;
			
			for (auto& pair : map)
			{
				if (pair.first.first > nl)
				{
					if (nl != -1)
						std::cout << '\n';
					
					nl = pair.first.first;
				}
				
				std::cout << pair.second << ' ';
			}

			std::cout << '\n' << std::endl;
		}
		
		MPI_Barrier(gridComm);
	}

	MPI_Comm_free(&gridComm);
	MPI_Finalize();
}