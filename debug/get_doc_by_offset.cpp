#include <vector>
#include <fstream>
#include <map>
#include <iostream>
#include <sstream>
#include <string.h>

#include "Configure.h"


using namespace std;

vector<pair<int, int> > offset_vec;

void open_file(ifstream &ifs, const char *filename)
{
    ifs.close();
    ifs.clear();
    ifs.open(filename);
    if (!(ifs.is_open()))
    {
        throw runtime_error("open file ");
    }
}

bool open_file(ofstream &ofs, const char *filename)
{
    ofs.close();
    ofs.clear();
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw runtime_error("open file ");
    }
}

void printDocById(int id,ifstream &in)
{
	// get offset and length
	in.seekg(0,in.beg);
	int offset = offset_vec[id-1].first;
	int length = offset_vec[id-1].second;
	// jump to offset
	in.seekg(offset,in.beg);
	// read doc in block;
	char *block = new char[length+1];
	//This function simply copies a block of data,
	//!!! without checking its contents nor appending a null character at the end.
	in.read(block,length);
	// so append NULL charactor at the end;
	block[length] = 0;

	string document(block);
	cout << block << endl;
	
	cout << "==================="<< endl<<endl;

	cout << "length is " << length << endl;
	cout << "document.size() is " << document.size()<<endl;


	cout << strcmp(block,document.c_str())<< endl;

	delete[] block;
}


void build_offset_vec(ifstream &ifs)
{
    string line;
    while (getline(ifs, line))
    {
        istringstream sl(line);
        int id, offset, length;
        sl >> id >> offset >> length;
        offset_vec.push_back(make_pair(offset,length));
    }
}

int main(int argc, char const *argv[])
{
    Configure *pconf = Configure::getInstance();
    string offset_path = pconf->getConfigByName("offset_path");
    string lib_path = pconf->getConfigByName("lib_path");

    ifstream libfile;
    open_file(libfile, offset_path.c_str());
    build_offset_vec(libfile);
    libfile.close();

    //open the ripelibfile

    ifstream ripefile;
    open_file(ripefile,lib_path.c_str());

    int id;
    cout << "enter a doc id---------------"<< endl;
    while (cin >> id)
    {
        printDocById(id,ripefile);
    	cout << "enter a doc id-------------"<< endl;
    }
    ripefile.close();
    return 0;
}