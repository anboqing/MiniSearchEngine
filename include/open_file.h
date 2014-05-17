#ifndef OPENFILE_H_____HHHHH___
#define OPENFILE_H_____HHHHH___

#include <fstream>
#include <stdexcept>

void open_file(std::ifstream &ifs, const char *filename)
{
    ifs.close();
    ifs.clear();
    ifs.open(filename);
    if (!(ifs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}

bool open_file(std::ofstream &ofs, const char *filename)
{
    ofs.close();
    ofs.clear();
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}
#endif