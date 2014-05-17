/*
    g++
*/
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Configure.h"

using namespace std;

int total_text = 0;

int docid = 0;

string gbk_titile;

vector<pair<int, int> > offsets;

void write_to_file(ofstream &ofs, const string &title, const char *url, const string &content)
{
    int offset = ofs.tellp();
    ofs << "<doc>" << endl;
    ofs << '\t' << "<docid>" << ++docid << "</docid>" << endl;
    ofs << '\t' << "<url>" << url << "</url>" << endl;
    ofs << '\t' << "<title>" << title << "</title>" << endl;
    ofs << '\t' << "<content>" << content << "</content>" << endl;
    ofs << "</doc>" << endl << endl;
    //update the offset; !!!! warning need to add tag's length;
    int length = ofs.tellp() - offset;
    // save the doc offset;
    offsets.push_back(make_pair(offset, length));
}


string get_titile(ifstream &infile)
{
    string title;
    bool has_default_title = false;
    infile.seekg (0, infile.beg);
    string line;
    int count = 10;
    size_t pos;
    while (getline(infile, line) && count--)
    {
        if ((pos = line.find(gbk_titile)) != string::npos)
        {
            title = line.substr(pos + gbk_titile.size(), line.size());
            has_default_title = true;
            break;
        }
    }

    if (!has_default_title)
    {
        infile.seekg (0, infile.beg);
        getline(infile, title);
    }

    return title;
}

void excute(const char *filename, ofstream &ofs,const char *url)
{
    //open file
    ifstream infile(filename, std::ifstream::binary);
    if (!(infile.is_open()))
    {
        throw runtime_error("open infile failed!");
    }
    // get length of file:
    infile.seekg (0, infile.end);
    int length = infile.tellg();
    infile.seekg (0, infile.beg);

    char *buffer = new char [length + 1];

    // read data as a block:
    infile.read(buffer, length);

    if (!(infile.is_open()))
    {
        std::cout << "error: only " << infile.gcount() << " could be read" << endl;
    }

    buffer[length] = '\0';
    // ...buffer contains the entire file...
    string content(buffer);
    //get title
    string title = get_titile(infile);

    string surl(url);
    string sfilename(filename);

    write_to_file(ofs,title,(surl+sfilename).c_str(),content);

    delete[] buffer;
    infile.close();
}

void traverseDir(const char *row_path, ofstream &ofs)
{

    DIR *dp = opendir(row_path);
    if (NULL == dp)
    {
        throw std::runtime_error("cannot open directory");
    }
    chdir(row_path);
    struct dirent *entry;
    struct stat statbuf;

    while ((entry = readdir(dp)) != NULL)
    {
        stat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode))
        {
            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;
            traverseDir(entry->d_name, ofs);
        }
        else
        {
            ++total_text;
            char dir_buf[512];
            bzero(dir_buf, 512);
            getcwd(dir_buf, 512);
            char url[512];  //记录url
            sprintf(url, "%s/%s", dir_buf, entry->d_name);
            excute(entry->d_name, ofs,url);
        }
    }
    chdir("..");
    closedir(dp);
}

void wirte_offsets_to_file(const char *filename);

int main(int argc, char const *argv[])
{

    Configure *pconf = Configure::getInstance();

    string row_path = pconf->getConfigByName("row_path");
    string lib_path = pconf->getConfigByName("lib_path");
    string title_path = pconf->getConfigByName("title_path");

    string offset_path = pconf->getConfigByName("offset_path");
    // get [biao ti]
    ifstream ifs(title_path.c_str());
    if (!ifs)
    {
        throw runtime_error("open title_path!");
    }
    getline(ifs, gbk_titile);
    ifs.close();
    // open ripepage.lib
    ofstream ofs(lib_path.c_str(), ofstream::out | ofstream::app);
    if (!(ofs.is_open()))
    {
        throw runtime_error("open lib_path failed");
    }
    // read file and build lib;
    traverseDir(row_path.c_str(), ofs);

    cout << "total_text is " << total_text << endl;
    ofs.close();

    wirte_offsets_to_file(offset_path.c_str());

    return 0;
}

void wirte_offsets_to_file(const char *filename)
{
    int docid = 0;
    ofstream ofs;
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw runtime_error("open offset file");
    }
    for (vector<pair<int, int> >::iterator iter = offsets.begin();
            iter != offsets.end(); ++iter)
    {
        ofs << ++docid << '\t' << (*iter).first << '\t' << (*iter).second << "\n";
    }
    ofs.close();
}
