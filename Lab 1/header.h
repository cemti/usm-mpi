#pragma once
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <ctime>
#include <cmath>
#include <map>
#include <mpi.h>
#include <cassert>
#include <climits>
#define ALLMAXLOC allMaxLoc

#define DECLARE_SIZES\
	int rows = ::rows, cols = ::cols;\
	if (flipped)\
		std::swap(rows, cols);

#define BEGIN_SYNC\
	for (int r = 0; r < nRanks; ++r) {\
		if (rank == r) {

#define END_SYNC\
		}\
		MPI_Barrier(MPI_COMM_WORLD);\
	}\
	MPI_Barrier(MPI_COMM_WORLD);

#define PRINT_MATRIX {\
	std::cout << "Matricea:\n";\
	for (int i = 0; i < rows; ++i) {\
		for (int j = 0; j < cols; ++j)\
			std::cout << mtx[i * cols + j] << ' ';\
		std::cout << '\n';\
	}\
	std::cout << std::endl;\
}

#define PRINT_PARTS {\
	std::cout << "Rankul " << rank << ":\n";\
	for (int i = 0; i < pInfo.part; ++i) {\
		for (int j = 0; j < cols; ++j)\
			std::cout << vec[i * cols + j] << ' ';\
		std::cout << '\n';\
	}\
	std::cout << std::endl;\
}

struct PartInfo
{
	// part este cele mai important
	int part, partSize, displ;
};

std::vector<float> matrices[2];
std::vector<int> offsets[2];
PartInfo partInfos[2];

std::set<std::pair<int, int>> points, ne;
int sizes[2], & rows = sizes[0], & cols = sizes[1], rank, nRanks;
bool independent, serial, verbose, loadBalancing;

MPI_Datatype floatIntArray[2];
MPI_Op allMaxLoc;