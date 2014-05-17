#ifndef LCS_H_
#define LCS_H_

#include <stdint.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

#include "EncodingConverter.h"

bool isGbkFirstByte(unsigned char c)
{
    return c >= 0x81 && c <= 0xFE;
}

void saveGbk2Vector(const std::string &str, std::vector<uint16_t> &v)
{
    v.clear();
    for (std::string::size_type ix = 0; (ix != str.size()) && (str.size() != 1); ++ix)
    {
        if (isGbkFirstByte(str[ix]))
        {
            if (ix + 1 == str.size())
            {
                throw std::runtime_error("gbk error");
            }
            unsigned char high = str[ix];
            unsigned char low = str[++ix];
            uint16_t whole = (high << 8) + low;
            v.push_back(whole);
        }
        else
        {
            v.push_back((uint16_t) (unsigned char)str[ix]);
        }
    }
}

int lcs(const std::vector<uint16_t>  &sa, const std::vector<uint16_t> &sb)
{

    int **matrix = new int *[sa.size() + 1];
    for (std::vector<uint16_t>::size_type i = 0; i <= sa.size(); ++i)
    {
        matrix[i] = new int[sb.size() + 1];
        memset(matrix[i], 0, sb.size() + 1);
    }

    for (std::vector<uint16_t>::size_type i = 0; i <= sa.size(); ++i)
    {
        matrix[i][0] = 0;
    }
    for (std::vector<uint16_t>::size_type i = 0; i <= sb.size(); ++i)
    {
        matrix[0][i] = 0;
    }

    for (std::vector<uint16_t>::size_type i = 1; i <= sa.size(); ++i)
    {
        for (std::vector<uint16_t>::size_type j = 1; j <= sb.size(); ++j)
        {
            if (sa[i - 1] == sb[j - 1])
            {
                matrix[i][j] = matrix[i - 1][j - 1] + 1;
            }
            else if (matrix[i - 1][j] >= matrix[i][j - 1] )
            {
                matrix[i][j] = matrix[i - 1][j];
            }
            else
            {
                matrix[i][j] = matrix[i][j - 1];
            }
        }
    }
    int ret = matrix[sa.size()][sb.size()];
    for (std::vector<uint16_t>::size_type i = 0; i <= sa.size(); ++i)
    {
        delete[] matrix[i];
    }
    delete matrix;
    return ret;
}

int getLongestCommonSubsequenceLength(const std::string &sa, const std::string &sb)
{
    std::vector<uint16_t> va, vb;
    saveGbk2Vector(sa, va);
    saveGbk2Vector(sb, vb);
    return lcs(va, vb);
}

#endif