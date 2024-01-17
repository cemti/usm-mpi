#include <iostream>
#include <algorithm>
#include <vector>
#include <ctime>
#include <climits>
#include <iomanip>
#include <cassert>
#include <mpi.h>

int sizes[2], subsizes[2], nRanks, wRank, rank;
MPI_Comm comm;
MPI_Datatype type;

void WriteMatrix()
{
    srand(time(NULL) ^ rank);

    MPI_File file;
    MPI_File_open(comm, "array.dat", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, type, "native", MPI_INFO_NULL);

    for (int i = 0; i < subsizes[0] * subsizes[1]; ++i)
    {
        int x = rand() & 65535;
        MPI_File_write(file, &x, 1, MPI_INT, MPI_STATUS_IGNORE);
    }

    MPI_File_close(&file);
}

void ReadMatrix()
{
    if (rank == 0)
    {
        MPI_File file;
        MPI_File_open(MPI_COMM_SELF, "array.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);

        std::cout << "Matricea:\n";

        for (int i = 0; i < sizes[0]; ++i)
        {
            for (int j = 0; j < sizes[1]; ++j)
            {
                int em;
                MPI_File_read(file, &em, 1, MPI_INT, MPI_STATUS_IGNORE);
                std::cout << std::setw(5) << em << ' ';
            }

            std::cout << '\n';
        }

        std::cout << std::endl;
        MPI_File_close(&file);
    }

    MPI_Barrier(comm);

    MPI_File file;
    MPI_File_open(comm, "array.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_set_view(file, 0, MPI_INT, type, "native", MPI_INFO_NULL);

    int mx = INT_MIN;

    for (int r = 0; r < nRanks; ++r)
    {
        if (rank == r && subsizes[0] > 0 && subsizes[1] > 0)
        {
            std::cout << "Rankul " << rank << " (" << wRank << " din MPI_COMM_WORLD):\n";

            for (int i = 0; i < subsizes[0]; ++i)
            {
                for (int j = 0; j < subsizes[1]; ++j)
                {
                    int em;
                    MPI_File_read(file, &em, 1, MPI_INT, MPI_STATUS_IGNORE);
                    mx = std::max(mx, em);
                    std::cout << std::setw(5) << em << ' ';
                }

                std::cout << '\n';
            }

            std::cout << "Max local: " << mx << '\n' << std::endl;
        }

        MPI_Barrier(comm);
    }

    MPI_File_close(&file);

    if (rank == 0)
    {
        MPI_Reduce(MPI_IN_PLACE, &mx, 1, MPI_INT, MPI_MAX, 0, comm);
        std::cout << "Max: " << mx << std::endl;
    }
    else
        MPI_Reduce(&mx, nullptr, 1, MPI_INT, MPI_MAX, 0, comm);
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &wRank);
    MPI_Comm_size(MPI_COMM_WORLD, &nRanks);

	if (argc < 7)
    {
        if (wRank == 0)
            std::cout << "Se cere sase parametri/trei dimensiuni pentru grila de procese, matrice, bloc." << std::endl;

        MPI_Finalize();
        return 0;
    }

    int gridDims[] = { atoi(argv[1]), atoi(argv[2]) },
        blocks[] = { atoi(argv[5]), atoi(argv[6]) };

    sizes[0] = atoi(argv[3]);
    sizes[1] = atoi(argv[4]);

    MPI_Comm_size(MPI_COMM_WORLD, &nRanks);
	
	if ((gridDims[0] != 0 || gridDims[1] != 0) && nRanks != gridDims[0] * gridDims[1])
    {
        gridDims[0] = gridDims[1] = 0;
		MPI_Dims_create(nRanks, 2, gridDims);
		
		if (wRank == 0)
            std::cout << "nr. de procese != aria grilei\nSe recalculeaza dimensiunile grilei de procese: " << gridDims[0] << ' ' << gridDims[1] << std::endl;

        MPI_Barrier(MPI_COMM_WORLD);
    }
    else
		MPI_Dims_create(nRanks, 2, gridDims);

	int periods[2] = {};

    for (int i = 0; i < 2; ++i)
    {
        {
            std::vector<int> ranks(nRanks);

            if (wRank == 0)
            {
                for (int i = 0; i < nRanks; ++i)
                    ranks[i] = i;

                std::random_shuffle(ranks.begin(), ranks.end());
            }

            MPI_Bcast(ranks.data(), nRanks, MPI_INT, 0, MPI_COMM_WORLD);

            MPI_Group wGroup, group;
            MPI_Comm rComm;

            MPI_Comm_group(MPI_COMM_WORLD, &wGroup);
            MPI_Group_incl(wGroup, nRanks, ranks.data(), &group);
            MPI_Comm_create(MPI_COMM_WORLD, group, &rComm);

            MPI_Cart_create(rComm, 2, gridDims, periods, 0, &comm);

            MPI_Group_free(&wGroup);
            MPI_Group_free(&group);
            MPI_Comm_free(&rComm);
        }

		MPI_Comm_rank(comm, &rank);

		int coords[2];
		MPI_Cart_coords(comm, rank, 2, coords);
		
		subsizes[0] = subsizes[1] = 0;

		for (int k = 0; k < 2; ++k)
			for (int i = blocks[k] * coords[k]; i < sizes[k]; i += gridDims[k] * blocks[k])
			{
				if (i + blocks[k] >= sizes[k])
				{
					subsizes[k] += sizes[k] - i;
					break;
				}

				subsizes[k] += blocks[k];
			}

		int distribs[] = { MPI_DISTRIBUTE_CYCLIC, MPI_DISTRIBUTE_CYCLIC };
		MPI_Type_create_darray(nRanks, rank, 2, sizes, distribs, blocks, gridDims, MPI_ORDER_C, MPI_INT, &type);
		MPI_Type_commit(&type);
		
		(i == 0 ? WriteMatrix : ReadMatrix)();
		
		MPI_Barrier(comm);
		MPI_Comm_free(&comm);
		MPI_Type_free(&type);
	}
	
    MPI_Finalize();
}