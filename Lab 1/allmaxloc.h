#pragma once
#include "header.h"

void FloatIntArrayAllMaxLoc(std::pair<float, int>* input, std::pair<float, int>* output, int* count, MPI_Datatype* datatype)
{
	int size;
	MPI_Type_size(*datatype, &size);

	size /= sizeof(std::pair<float, int>);

	for (int i = 0; i < *count; ++i)
	{
		int idx = i * size;

		if (input[idx].first > output[idx].first)
		{
			output[idx] = input[idx];

			for (int j = 1; j < size; ++j)
			{
				if (input[idx].first != input[idx + j].first)
					break;

				output[idx + j] = input[idx + j];
			}
		}
		else if (input[idx].first == output[idx].first && input[idx].first != -INFINITY)
			for (int j = 1; j < size; ++j)
				if (output[idx].first != output[idx + j].first)
				{
					for (int k = 0; k < size; ++k)
					{
						if (input[idx].first != input[idx + k].first)
							break;

						output[idx + j + k] = input[idx + k];
					}

					break;
				}
	}
}

void ProcessMatrix(bool flipped)
{
	DECLARE_SIZES

	auto& pInfo = partInfos[flipped];
	std::vector<std::pair<float, int>> inflatedMtx(pInfo.part * cols * pInfo.part, { -INFINITY, -1 });

	{
		auto& vec = matrices[flipped];
		auto& offsetVec = offsets[flipped];

		for (int i = 0; i < pInfo.part; ++i)
			for (int j = 0; j < cols; ++j)
				inflatedMtx[i * cols * pInfo.part + j * pInfo.part] = { vec[i * cols + j], offsetVec[i] };

		MPI_Datatype tempFloatIntArray;
		MPI_Type_contiguous(pInfo.part, MPI_FLOAT_INT, &tempFloatIntArray);

		for (int i = 1; i < pInfo.part; ++i)
			FloatIntArrayAllMaxLoc(inflatedMtx.data() + i * cols * pInfo.part, inflatedMtx.data(), &cols, &tempFloatIntArray);

		MPI_Type_free(&tempFloatIntArray);
	}

	if (pInfo.part > 0)
	{
		inflatedMtx.resize(inflatedMtx.size() / pInfo.part);
		
		auto oldInflatedVec(std::move(inflatedMtx));

		inflatedMtx.clear();
		inflatedMtx.resize(cols * rows, { -INFINITY, -1 });

		for (int i = 0; i < cols; ++i)
		{
			if (oldInflatedVec[i * pInfo.part].first == -INFINITY)
				continue;

			inflatedMtx[i * rows] = oldInflatedVec[i * pInfo.part];
			
			for (int j = 1; j < pInfo.part; ++j)
			{
				if (oldInflatedVec[i * pInfo.part].first != oldInflatedVec[i * pInfo.part + j].first)
					break;
				
				inflatedMtx[i * rows + j] = oldInflatedVec[i * pInfo.part + j];
			}
		}
	}
	else
		inflatedMtx.resize(cols * rows, { -INFINITY, -1 });

	if (rank == 0)
	{
		std::vector<std::pair<float, int>> reducedVec(cols * rows);
		MPI_Reduce(inflatedMtx.data(), reducedVec.data(), cols, floatIntArray[flipped], ALLMAXLOC, 0, MPI_COMM_WORLD);

		if (verbose)
		{
			std::cout << "Elementele maximale de pe coloane:\n";

			for (int i = 0; i < cols; ++i)
			{
				std::cout << "Coloana " << i << ": ";

				for (int j = 0; j < rows; ++j)
				{
					auto& cell = reducedVec[i * rows + j];

					if (j > 0 && cell.first != reducedVec[i * rows + j - 1].first)
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
				for (int j = 0; j < rows; ++j)
				{
					auto& cell = reducedVec[i * rows + j];

					if (j > 0 && cell.first != reducedVec[i * rows + j - 1].first)
						break;

					auto point = flipped ? std::pair<int, int>(i, cell.second) : std::pair<int, int>(cell.second, i);

					if (points.count(point))
						ne.insert(point);
					else
						points.insert(point);
				}
	}
	else
		MPI_Reduce(inflatedMtx.data(), nullptr, cols, floatIntArray[flipped], ALLMAXLOC, 0, MPI_COMM_WORLD);
}