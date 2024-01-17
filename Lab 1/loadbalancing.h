#pragma once
#include "header.h"

void ReadMatrixLB(std::istream& in, bool flipped)
{
	DECLARE_SIZES

	std::map<int, std::vector<float>> slots;
	int turn = 0;
	float temp;

	if (flipped)
	{
		int bTurn = 0;

		for (int i = 0; i < cols; ++i)
		{
			turn = bTurn;

			for (int j = 0; j < rows; ++j)
			{
				in >> temp;

				if (turn == rank)
				{
					if (slots.count(j) == 0)
						slots[j].resize(cols, -INFINITY);

					slots[j][i] = temp;
				}

				turn = (turn + cols) % nRanks;
			}

			++bTurn;

			if (bTurn == nRanks)
				bTurn = 0;
		}
	}
	else
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
			{
				in >> temp;

				if (turn == rank)
				{
					if (slots.count(i) == 0)
						slots[i].resize(cols, -INFINITY);

					slots[i][j] = temp;
				}

				++turn;

				if (turn == nRanks)
					turn = 0;
			}

	auto& vec = matrices[flipped];
	auto& pInfo = partInfos[flipped];
	auto& offsetVec = offsets[flipped];

	for (auto& pair : slots)
	{
		offsetVec.push_back(pair.first);
		vec.insert(vec.end(), pair.second.begin(), pair.second.end());
	}

	pInfo.part = int(vec.size()) / cols;

	if (verbose)
	{
		BEGIN_SYNC

		if (rank < rows * cols)
			PRINT_PARTS

		END_SYNC
	}
}

void DistributeMatrixLB(const float* mtx, bool flipped)
{
	DECLARE_SIZES

	if (rank == 0 && verbose)
		PRINT_MATRIX

	auto& vec = matrices[flipped];
	auto& pInfo = partInfos[flipped];
	auto& offsetVec = offsets[flipped];

	const int area = rows * cols;
	int cellCount, vecOffset = -1;

	if (nRanks >= area)
		cellCount = rank < area ? 1 : 0;
	else
	{
		cellCount = area / nRanks;

		if (rank + 1 <= area % nRanks)
			++cellCount;
	}

	if (rank == 0)
	{
		std::vector<int> counts(nRanks), displs(nRanks);
		MPI_Gather(MPI_IN_PLACE, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		int n = 0;
		MPI_Reduce(MPI_IN_PLACE, &n, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		std::vector<int> cellCounts((nRanks < area ? nRanks : area) - 1);
		std::vector<std::pair<int, int>> positions(n);
		std::vector<float> values(n);
		int turn = 0;

		for (int i = 0; i < cellCounts.size(); ++i)
			displs[i + 1] = displs[i] + counts[i];

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				if (turn == rank)
				{
					if (offsetVec.size() == 0 || i > *offsetVec.rbegin())
					{
						vecOffset = int(vec.size());
						vec.resize(vecOffset + cols, -INFINITY);
						offsetVec.push_back(i);
					}

					vec[vecOffset + j] = mtx[i * cols + j];
				}
				else
				{
					int displ = displs[turn], & idx = cellCounts[turn - 1];
					positions[displ + idx] = { i, j };
					values[displ + idx] = mtx[i * cols + j];
					++idx;
				}

				++turn;

				if (turn == nRanks)
					turn = 0;
			}
		}

		MPI_Scatterv(positions.data(), counts.data(), displs.data(), MPI_2INT, nullptr, 0, MPI_2INT, 0, MPI_COMM_WORLD);
		MPI_Scatterv(values.data(), counts.data(), displs.data(), MPI_FLOAT, nullptr, 0, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}
	else
	{
		MPI_Gather(&cellCount, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Reduce(&cellCount, nullptr, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

		std::vector<std::pair<int, int>> positions(cellCount);
		std::vector<float> values(cellCount);

		MPI_Scatterv(nullptr, nullptr, nullptr, MPI_2INT, positions.data(), cellCount, MPI_2INT, 0, MPI_COMM_WORLD);
		MPI_Scatterv(nullptr, nullptr, nullptr, MPI_FLOAT, values.data(), cellCount, MPI_FLOAT, 0, MPI_COMM_WORLD);

		for (int i = 0; i < cellCount; ++i)
		{
			auto& pos = positions[i];

			if (offsetVec.size() == 0 || pos.first > *offsetVec.rbegin())
			{
				vecOffset = int(vec.size());
				vec.resize(vecOffset + cols, -INFINITY);
				offsetVec.push_back(pos.first);
			}

			vec[vecOffset + pos.second] = values[i];
		}
	}

	pInfo.part = int(vec.size()) / cols;

	if (verbose)
	{
		BEGIN_SYNC

		if (rank < rows * cols)
			PRINT_PARTS

		END_SYNC
	}
}