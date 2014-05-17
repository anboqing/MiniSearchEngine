#include "Document.h"
#include "Configure.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

void open_file(ifstream &ifs,const char *filename){
	ifs.close();
	ifs.clear();
	ifs.open(filename);
	if(!(ifs.is_open())){
		throw runtime_error("open file ");
	}
}

bool open_file(ofstream &ofs,const char *filename){
	ofs.close();
	ofs.clear();
	ofs.open(filename);
	if(!(ifs.is_open())){
		throw runtime_error("open file ");
	}
}

void parse(vector<Document> &doc_vec,ifstream &ifs){
	Document adoc;
	
}

int main(int argc, char const *argv[])
{
	Configure *pconf = Configure::getInstance();
	string lib_path = pconf->getConfigByName("lib_path");
	ifstream ifs;
	open_file(ifs,lib_path.c_str());
	
	vector<Document> doc_vec;

	parse(doc_vec,ifs);

	
	return 0;
}