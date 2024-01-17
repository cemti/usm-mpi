#pragma once
#include "independent.h"

void DistributeMatrix(const float* mtx, bool flipped)
{
	DECLARE_SIZES

	std::vector<int> sizes, displs;

	if (rank == 0)
	{
		if (verbose)
			PRINT_MATRIX

		sizes.resize(nRanks);
		displs.resize(nRanks);
	}

	auto& vec = matrices[flipped];
	auto& pInfo = partInfos[flipped];

	CalculatePartData(flipped);
	MPI_Gather(&pInfo.partSize, 1, MPI_INT, sizes.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Gather(&pInfo.displ, 1, MPI_INT, displs.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

	vec.resize(pInfo.part * cols);
	MPI_Scatterv(mtx, sizes.data(), displs.data(), MPI_FLOAT, vec.data(), int(vec.size()), MPI_FLOAT, 0, MPI_COMM_WORLD);

	if (verbose)
	{
		BEGIN_SYNC

		if (rank < rows)
			PRINT_PARTS

		END_SYNC
	}
}

void PrepareMatrices(const char* name)
{
	if (independent)
	{
		std::ifstream fin(name);
		fin >> rows >> cols;

		auto ptr = loadBalancing ? ReadMatrixLB : ReadMatrix;
		ptr(fin, false);
		ptr(fin, true);
	}
	else
	{
		auto ptr = loadBalancing ? DistributeMatrixLB : DistributeMatrix;

		if (rank == 0)
		{
			std::ifstream fin(name);
			fin >> rows >> cols;

			std::vector<float> A(rows * cols), Btr(rows * cols);

			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					fin >> A[i * cols + j];

			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
					fin >> Btr[j * rows + i];

			MPI_Bcast(sizes, 2, MPI_INT, 0, MPI_COMM_WORLD);
			ptr(A.data(), false);
			ptr(Btr.data(), true);
		}
		else
		{
			MPI_Bcast(sizes, 2, MPI_INT, 0, MPI_COMM_WORLD);
			ptr(nullptr, false);
			ptr(nullptr, true);
		}
	}
}