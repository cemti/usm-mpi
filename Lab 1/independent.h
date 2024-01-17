#pragma once
#include "header.h"

void CalculatePartData(bool flipped)
{
	DECLARE_SIZES

	auto& pInfo = partInfos[flipped];
	int initialPart = rows / nRanks;

	if (initialPart == 0 && rank < rows)
		initialPart = 1;

	pInfo.part = initialPart;
	pInfo.partSize = pInfo.part * cols;
	pInfo.displ = rank * pInfo.partSize;

	if (pInfo.part > 0 && rank + 1 == nRanks)
	{
		pInfo.part = rows - rank * pInfo.part;
		pInfo.partSize = pInfo.part * cols;
	}

	for (int i = 0; i < pInfo.part; ++i)
		offsets[flipped].push_back(initialPart * rank + i);
}

void ReadMatrix(std::istream& in, bool flipped)
{
	DECLARE_SIZES

	auto& vec = matrices[flipped];
	auto& pInfo = partInfos[flipped];

	CalculatePartData(flipped);

	if (pInfo.part > 0)
	{
		int lineOffset = offsets[flipped][0];
		float temp;

		vec.resize(pInfo.part * cols);

		if (flipped)
		{
			for (int i = 0; i < cols; ++i)
				for (int j = 0; j < rows; ++j)
				{
					in >> temp;

					if (j >= lineOffset && j < lineOffset + pInfo.part)
						vec[(j - lineOffset) * cols + i] = temp;
				}
		}
		else
		{
			int idx = 0;

			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j)
				{
					in >> temp;

					if (i >= lineOffset && i < lineOffset + pInfo.part)
						vec[idx++] = temp;
				}
		}
	}

	if (verbose)
	{
		BEGIN_SYNC

		if (rank < rows)
			PRINT_PARTS

		END_SYNC
	}
}

void ProcessMatrixSerial(bool flipped)
{
	DECLARE_SIZES

	auto& pInfo = partInfos[flipped];
	auto& vec = matrices[flipped];
	auto& offsetVec = offsets[flipped];

	std::vector<std::vector<std::pair<float, int>>> reducedVec(cols);

	for (int i = 0; i < pInfo.part; ++i)
		for (int j = 0; j < cols; ++j)
		{
			auto value = vec[i * cols + j];

			if (value == -INFINITY)
				continue;

			auto& v = reducedVec[j];
			int idx = offsetVec[i];

			if (v.empty() || value == v[0].first)
				v.emplace_back(value, idx);
			else if (value > v[0].first)
			{
				v.resize(1);
				v[0] = { value, idx };
			}
		}

	if (rank == 0)
	{
		std::vector<int> counts(nRanks), displs(nRanks);

		MPI_Gather(MPI_IN_PLACE, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

		for (int i = 1; i < nRanks; ++i)
			displs[i] = displs[i - 1] + counts[i - 1];

		std::vector<std::pair<int, int>> positions(displs[nRanks - 1] + counts[nRanks - 1]);
		std::vector<float> values(positions.size());

		MPI_Gatherv(nullptr, 0, MPI_2INT, positions.data(), counts.data(), displs.data(), MPI_2INT, 0, MPI_COMM_WORLD);
		MPI_Gatherv(nullptr, 0, MPI_FLOAT, values.data(), counts.data(), displs.data(), MPI_FLOAT, 0, MPI_COMM_WORLD);

		for (int i = 1; i < nRanks; ++i)
		{
			int n = counts[i], offset = displs[i];

			for (int i = 0; i < n; ++i)
			{
				auto& p = positions[offset + i];
				auto& v = reducedVec[p.second];
				auto value = values[offset + i];

				if (v.empty() || value == v[0].first)
					v.emplace_back(value, p.first);
				else if (value > v[0].first)
				{
					v.resize(1);
					v[0] = { value, p.first };
				}
			}
		}
	}
	else
	{
		std::vector<std::pair<int, int>> positions;
		std::vector<float> values;

		for (int i = 0; i < cols; ++i)
			for (auto& p : reducedVec[i])
			{
				positions.push_back({ p.second, i });
				values.push_back(p.first);
			}

		int n = int(values.size());
		MPI_Gather(&n, 1, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gatherv(positions.data(), n, MPI_2INT, nullptr, nullptr, nullptr, MPI_2INT, 0, MPI_COMM_WORLD);
		MPI_Gatherv(values.data(), n, MPI_FLOAT, nullptr, nullptr, nullptr, MPI_FLOAT, 0, MPI_COMM_WORLD);
	}

	if (rank == 0)
	{
		if (verbose)
		{
			std::cout << "Elementele maximale de pe coloane:\n";

			for (int i = 0; i < cols; ++i)
			{
				std::cout << "Coloana " << i << ": ";

				for (int j = 0; j < reducedVec[i].size(); ++j)
				{
					auto& cell = reducedVec[i][j];

					if (j > 0 && cell.first != reducedVec[i][j - 1].first)
						break;

					std::cout << cell.first << " @ (" << cell.second << ", " << i << "), ";

					auto point = flipped ? std::pair<int, int>(i, cell.second) : std::pair<int, int>(cell.second, i);

					if (points.count(point))
						ne.insert(point);
					else
						points.insert(point);
				}

				std::cout << '\n';
			}

			std::cout << std::endl;
		}
		else
			for (int i = 0; i < cols; ++i)
				for (int j = 0; j < reducedVec[i].size(); ++j)
				{
					auto& cell = reducedVec[i][j];

					if (j > 0 && cell.first != reducedVec[i][j - 1].first)
						break;

					auto point = flipped ? std::pair<int, int>(i, cell.second) : std::pair<int, int>(cell.second, i);

					if (points.count(point))
						ne.insert(point);
					else
						points.insert(point);
				}
	}
}