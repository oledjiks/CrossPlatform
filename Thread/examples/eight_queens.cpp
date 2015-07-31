#include <iostream>
#include <sstream>
#include <vector>
#include <thread>

#if defined __WIN32 || defined __WIN64 || defined WIN32 || defined WIN64
#define WINDOWS
#endif

#ifdef WINDOWS
#include <windows.h>
#endif

#include "../ThreadPool.h"

#define MAX_BOARDSIZE 2048
#define BOARDSIZE     8
#define THREAD_NUM    4

using namespace std;

double get_current_performance_timer()
{
#ifdef WINDOWS
    LARGE_INTEGER large_interger;
    double freq, c;
    QueryPerformanceFrequency(&large_interger);
    freq = large_interger.QuadPart;
    QueryPerformanceCounter(&large_interger);
    c = large_interger.QuadPart;

    return (c / freq);
#else
    return clock();
#endif
}

void place_queens_multithread(size_t board_size = BOARDSIZE, unsigned int thread_num = THREAD_NUM);

class Queens
{
    friend void place_queens_multithread(size_t, unsigned int);
private:
    vector<int> _chessboard;
    size_t      _board_size;
    vector<int> _column_flag, _slash1_flag, _slash2_flag;

    void _print_board(ostream& os)
    {
        stringstream ss;
        for (auto pos : _chessboard)
            ss << string(2*pos, '_') << "##" << string(2*(_board_size - pos - 1), '_') << "\n";
        ss << "\n";
        os << ss.str();
    };

    void _place_nrow_queen(int row, ostream& os)
    {
        for (size_t col = 0; col < _board_size; ++col)
        {
            if (_column_flag[col] == 1
                || _slash1_flag[row + col] == 1
                || _slash2_flag[row - col + _board_size - 1] == 1)
                continue;

            _chessboard[row] = col;
            _column_flag[col] = 1;
            _slash1_flag[row + col] = 1;
            _slash2_flag[row - col + _board_size - 1] = 1;

            if (row > 0)
                _place_nrow_queen(row - 1, cout);
            else
                _print_board(os);

            // backtrak
            _chessboard[row] = -1;
            _column_flag[col] = 0;
            _slash1_flag[row + col] = 0;
            _slash2_flag[row - col + _board_size - 1] = 0;
        }
    };

public:
    Queens(size_t board_size = BOARDSIZE)
    {
        if (board_size > MAX_BOARDSIZE)
            _board_size = MAX_BOARDSIZE;
        else
            _board_size = board_size;
        _chessboard.resize(board_size, -1);
        _column_flag.resize(board_size, 0);
        _slash1_flag.resize(2*board_size - 1, 0);
        _slash2_flag.resize(2*board_size - 1, 0);
    };

    virtual ~Queens() {};

    void place_all_queens()
    {
        // _place_nrow_queen(_board_size - 1, cout);

        for (unsigned int col = 0; col < _board_size; ++col)
        {
            _chessboard[_board_size - 1] = col;
            _column_flag[col] = 1;
            _slash1_flag[_board_size - 1 + col] = 1;
            _slash2_flag[_board_size - 1 - col + _board_size - 1] = 1;

            _place_nrow_queen(_board_size - 2, cout);

            _chessboard[_board_size - 1] = -1;
            _column_flag[col] = 0;
            _slash1_flag[_board_size - 1 + col] = 0;
            _slash2_flag[_board_size - 1 - col + _board_size - 1] = 0;
        }
    };
};

void place_queens_multithread(size_t board_size, unsigned int thread_num)
{
    ThreadPool      pool(thread_num);

    for (unsigned int col = 0; col < board_size; ++col)
    {
        pool.enqueue([&] () {
                Queens queens(board_size);
                queens._chessboard[board_size - 1] = col;
                queens._column_flag[col] = 1;
                queens._slash1_flag[board_size - 1 + col] = 1;
                queens._slash2_flag[board_size - 1 - col + board_size - 1] = 1;

                queens._place_nrow_queen(board_size - 2, cout);
            });
    }
};


int main(int argc, char *argv[])
{
    int n;
    cout << "input n: ";
    cin >> n;
    auto start1 = get_current_performance_timer();
    Queens test_queens(n);
    test_queens.place_all_queens();
    auto end1 = get_current_performance_timer();

    cout << "input n: ";
    cin >> n;
    auto start2 = get_current_performance_timer();
    place_queens_multithread(n, 8);
    auto end2 = get_current_performance_timer();

    cout << "time: " << end1 - start1 << endl;
    cout << "time: " << end2 - start2 << endl;

    return 0;
}
