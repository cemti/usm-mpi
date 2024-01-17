#include <iostream>
#include <fstream>
#include <ctime>

int A[500 * 500], B[500 * 500];

int main()
{
    srand(time(NULL));
    
    for (int i = 0; i < 500 * 500; ++i)
    {
        A[i] = rand() % 100;
        B[i] = rand() % 100;
    }
    
    for (int i = 0; i < 32; ++i)
    {
        int x = rand() % 500, y = rand() % 500;
        
        A[x * 500 + y] = (rand() % 100) + 100;
        B[x * 500 + y] = (rand() % 100) + 100;
    }
    
    std::ofstream fout("a.txt");
    
    fout << 500 << ' ' <<  500 << '\n';
    
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 500; ++j)
        {
            fout << A[i * 500 + j] << ' ';
        }
        
        fout << "\n";
    }
    
    fout << "\n";
    
    for (int i = 0; i < 500; ++i)
    {
        for (int j = 0; j < 500; ++j)
        {
            fout << B[i * 500 + j] << ' ';
        }
        
        fout << "\n";
    }
}
