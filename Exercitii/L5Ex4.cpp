#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <set>
#include <mpi.h>
#define MPI_ALLMAXLOC allMaxLoc

struct IntInt
{
	int value, index;
};

MPI_Datatype twoIntsArray;
MPI_Op allMaxLoc;

void AllMaxLoc(void* input, void* output, int* count, MPI_Datatype* datatype)
{
	assert(*datatype == twoIntsArray);

	IntInt* inputPairs = (IntInt*)input, * outputPairs = (IntInt*)output;

	int size;
	MPI_Type_size(twoIntsArray, &size);

	size /= sizeof(IntInt);

	for (int i = 0; i < *count; ++i)
	{
		int idx = i * size;

		if (inputPairs[idx].value > outputPairs[idx].value)
		{
			outputPairs[idx] = inputPairs[idx];

			for (int j = 1; j < size; ++j)
			{
				if (inputPairs[idx].value != inputPairs[idx + j].value)
					break;

				outputPairs[idx + j] = inputPairs[idx + j];
			}
		}
		else if (inputPairs[idx].value == outputPairs[idx].value)
		{
			for (int j = 1; j < size; ++j)
			{
				if (outputPairs[idx + j].value != outputPairs[idx].value)
				{
					for (int k = 0; k < size; ++k)
					{
						if (inputPairs[idx + k].value != inputPairs[idx].value)
							break;

						outputPairs[idx + j + k] = inputPairs[idx + k];
					}

					break;
				}
			}
		}
	}
}

int main()
{
	MPI_Init(NULL, NULL);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	int ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &ranks);

	MPI_Type_contiguous(ranks, MPI_2INT, &twoIntsArray);
	MPI_Type_commit(&twoIntsArray);

	MPI_Op_create(AllMaxLoc, false, &MPI_ALLMAXLOC);

	std::vector<IntInt> mtx;

	if (rank == 0)
	{
		std::cout << "Matricea initiala:\n";
		mtx.resize(ranks * ranks);
		int seed = time(NULL);
		srand(seed);
		int seedTrunc = rand() & 3;

		for (int i = 0; i < ranks; ++i)
		{
			srand(seed + (i & seedTrunc));
			
			for (int j = 0; j < ranks; ++j)
			{
				IntInt& cell = mtx[i * ranks + j];

				cell.value = rand();
				cell.index = i;

				std::cout << cell.value << ' ';
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
	}

	std::vector<IntInt> line(ranks);
	MPI_Scatter(mtx.data(), ranks, MPI_2INT, line.data(), line.size(), MPI_2INT, 0, MPI_COMM_WORLD);

	for (int i = ranks; i > 0; --i)
		line.insert(line.begin() + i, ranks - 1, IntInt());

	if (rank == 0)
	{
		std::vector<IntInt> maxs(ranks * ranks);
		MPI_Reduce(line.data(), maxs.data(), ranks, twoIntsArray, MPI_ALLMAXLOC, 0, MPI_COMM_WORLD);

		std::cout << "Elementele maximale de pe coloane (valoare, rank/linia):\n";

		for (int i = 0; i < ranks; ++i)
		{
			std::cout << "Coloana " << i << ": ";
			
			for (int j = 0; j < ranks; ++j)
			{
				IntInt& cell = maxs[i * ranks + j];

				if (j > 0 && cell.value != maxs[i * ranks + j - 1].value)
					break;

				std::cout << '(' << cell.value << ", " << cell.index << "), ";
			}

			std::cout << '\n';
		}

		std::cout << std::endl;
	}
	else
		MPI_Reduce(line.data(), NULL, ranks, twoIntsArray, MPI_ALLMAXLOC, 0, MPI_COMM_WORLD);

	MPI_Op_free(&MPI_ALLMAXLOC);
	MPI_Type_free(&twoIntsArray);
	MPI_Finalize();
}
