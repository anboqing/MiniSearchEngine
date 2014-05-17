#include <string>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
	string line("【 标  题 】南非金矿脉的发");
	if(line.find("【 标  题 】")==0){
		cout << line.c_str()+16;
	}
	return 0;
}