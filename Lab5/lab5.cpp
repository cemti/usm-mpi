#include <iostream>
#include <fstream>
#include <ctime>
#include <climits>
#include <cstring>
#include <vector>
#include <tuple>
#include <set>
#include <map>
#include <mpi.h>
#include <sstream>
#include <memory>
static_assert(sizeof(std::tuple<int, int, int>) == 3 * sizeof(int), "sizeof(Tuple) != 12");

int gridDims[2], sizes[2], blocks[2], coords[2], nRanks, rank;
std::set<std::pair<int, int>> points, ne;
const char* fileName;
MPI_Comm gridComm;
MPI_Datatype fType, threeInt;
MPI_File arrayFile;
bool print;

void PrepareMatrix()
{
    std::ifstream fin(fileName);
    fin >> sizes[0] >> sizes[1];

    if (rank != 0)
        return;

    std::ofstream fout("array.dat", std::ios::binary);

    int n = sizes[0] * sizes[1] << 1;
    
    for (int i = 0; i < n; ++i)
    {
        int temp;
        fin >> temp;
        // mai performant decat MPI_File_write
        fout.write(reinterpret_cast<char*>(&temp), sizeof(int));
    }
}

void ProcessMatrixSerial(bool transposed)
{
    std::map<int, std::vector<std::tuple<int, int, int>>> mxs;

    {
        int rowC = 0;
        std::unique_ptr<std::ostringstream> str;

        if (print)
            str.reset(new std::ostringstream);
        
        for (int i = blocks[0] * coords[0]; i < sizes[0]; ++i)
        {
            int colC = 0;
            
            for (int j = blocks[1] * coords[1]; j < sizes[1]; ++j)
            {
                int value;
                MPI_File_read(arrayFile, &value, 1, MPI_INT, MPI_STATUS_IGNORE);

                if (print)
                    (*str) << value << ' ';

                auto& v = mxs[transposed ? i : j];

                std::tuple<int, int, int> temp(value, i, j);

                if (v.empty() || value == std::get<0>(v[0]))
                    v.push_back(temp);
                else if (value > std::get<0>(v[0]))
                {
                    v.resize(1);
                    v[0] = temp;
                }

                if (++colC >= blocks[1])
                {
                    j += blocks[1] * (gridDims[1] - 1);
                    colC = 0;
                }
            }

            if (print)
                (*str) << '\n';

            if (++rowC >= blocks[0])
            {
                i += blocks[0] * (gridDims[0] - 1);
                rowC = 0;
            }
        }

        if (print)
        {
            for (int i = 0; i < nRanks; ++i)
            {
                if (rank == i)
                {
                    std::cout << "Rankul " << i << "@(" << coords[0] << ',' << coords[1] << "):\n" << str->str();

                    for (auto& pair : mxs)
                    {
                        std::cout << (transposed ? "Randul" : "Coloana") << ' ' << pair.first << ": ";

                        for (auto& p : pair.second)
                            std::cout << std::get<0>(p) << "@(" << std::get<1>(p) << ',' << std::get<2>(p) << "), ";

                        std::cout << '\n';
                    }

                    std::cout << std::endl;
                }
                
                MPI_Barrier(gridComm);
            }
        }
    }

    if (rank == 0)
    {
        std::vector<int> counts(nRanks), displs(nRanks);

        MPI_Gather(MPI_IN_PLACE, 1, MPI_INT, counts.data(), 1, MPI_INT, 0, gridComm);

        for (int i = 1; i < nRanks; ++i)
            displs[i] = displs[i - 1] + counts[i - 1];

        std::vector<std::tuple<int, int, int>> ems(displs[nRanks - 1] + counts[nRanks - 1]);

        MPI_Gatherv(nullptr, 0, threeInt, ems.data(), counts.data(), displs.data(), threeInt, 0, gridComm);

        for (int i = 1; i < nRanks; ++i)
        {
            int n = counts[i], offset = displs[i];

            for (int i = 0; i < n; ++i)
            {
                auto& t = ems[offset + i];
                int value = std::get<0>(t), row = std::get<1>(t), col = std::get<2>(t);

                auto& v = mxs[transposed ? row : col];

                std::tuple<int, int, int> temp(value, row, col);

                if (v.empty() || value == std::get<0>(v[0]))
                    v.push_back(temp);
                else if (value > std::get<0>(v[0]))
                {
                    v.resize(1);
                    v[0] = temp;
                }
            }
        }

        for (auto& pair : mxs)
            while (!pair.second.empty())
            {
                auto& p = pair.second.back();
                std::pair<int, int> point(std::get<1>(p), std::get<2>(p));

                if (!transposed)
                    points.insert(point);
                else if (points.count(point) > 0)
                {
                    ne.insert(point);
                    points.erase(point);
                }

                pair.second.pop_back();
            }
    }
    else
    {
        std::vector<std::tuple<int, int, int>> ems;

        for (auto& v : mxs)
            while (!v.second.empty())
            {
                ems.push_back(v.second.back());
                v.second.pop_back();
            }

        int n = int(ems.size());
        MPI_Gather(&n, 1, MPI_INT, NULL, 0, MPI_INT, 0, gridComm);
        MPI_Gatherv(ems.data(), n, threeInt, nullptr, nullptr, nullptr, threeInt, 0, gridComm);
    }
}

int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);

    MPI_Type_contiguous(3, MPI_INT, &threeInt);
    MPI_Type_commit(&threeInt);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 6)
    {
        if (rank == 0)
            std::cout << "Se cere cinci parametri (fisierul, dim. grilei de procese, dim. blocului)." << std::endl;

        MPI_Finalize();
        return 0;
    }

    bool timed = false, silent = false;

    for (int i = 6; i < argc; ++i)
    {
        if (strcmp(argv[i], "time") == 0)
            timed = true;
        else if (strcmp(argv[i], "silent") == 0)
            silent = true;
        else if (strcmp(argv[i], "print") == 0)
            print = true;
    }

    fileName = argv[1];

    gridDims[0] = atoi(argv[2]);
    gridDims[1] = atoi(argv[3]);

    blocks[0] = atoi(argv[4]);
    blocks[1] = atoi(argv[5]);

    auto fileTimestamp = MPI_Wtime();
    PrepareMatrix();
    fileTimestamp = MPI_Wtime() - fileTimestamp;
    
    MPI_Comm_size(MPI_COMM_WORLD, &nRanks);

    if (nRanks != gridDims[0] * gridDims[1] && (gridDims[0] != 0 || gridDims[1] != 0))
    {
        if (rank == 0)
            std::cout << "nr. de procese != aria grilei." << std::endl;

        MPI_Finalize();
        return 0;
    }

    {
        int periods[2] = {};
        MPI_Dims_create(nRanks, 2, gridDims);
        MPI_Cart_create(MPI_COMM_WORLD, 2, gridDims, periods, 0, &gridComm);
    }

    MPI_Cart_coords(gridComm, rank, 2, coords);

    {
        int distribs[] = { MPI_DISTRIBUTE_CYCLIC, MPI_DISTRIBUTE_CYCLIC };
        MPI_Type_create_darray(nRanks, rank, 2, sizes, distribs, blocks, gridDims, MPI_ORDER_C, MPI_INT, &fType);
        MPI_Type_commit(&fType);
    }

    MPI_File_open(gridComm, "array.dat", MPI_MODE_RDONLY, MPI_INFO_NULL, &arrayFile);
    MPI_File_set_view(arrayFile, 0, MPI_BYTE, fType, "native", MPI_INFO_NULL);

    double timestamp = MPI_Wtime();
    ProcessMatrixSerial(false);

    if (rank == 0 && print)
        std::cout << "----" << std::endl;

    ProcessMatrixSerial(true);
    timestamp = MPI_Wtime() - timestamp;

    if (rank == 0 && !silent)
    {
        std::cout << "Situatiile Nash de echilibru:\n";

        if (ne.empty())
            std::cout << "Nu exista nici una.";
        else
        {
            for (auto& pair : ne)
                std::cout << "(" << pair.first << ", " << pair.second << "), ";

            std::cout << "\nTotal: " << ne.size();
        }

        std::cout << std::endl;
    }

    if (timed)
    {
        if (rank == 0)
        {
            double avg, mx, mn;

            MPI_Reduce(&timestamp, &avg, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&timestamp, &mx, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&timestamp, &mn, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);

            std::cout << "Crearea fisierului: " << fileTimestamp
                << "\nMedia: " << avg / nRanks << ", Max: " << mx << ", Min: " << mn << std::endl;
        }
        else
        {
            MPI_Reduce(&timestamp, nullptr, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
            MPI_Reduce(&timestamp, nullptr, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
            MPI_Reduce(&timestamp, nullptr, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
        }
    }

    MPI_File_close(&arrayFile);
    MPI_Comm_free(&gridComm);
    MPI_Type_free(&fType);
    MPI_Type_free(&threeInt);
    MPI_Finalize();
}