#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;

string extractFingerPrint(const string &content_)
{
    // string comma = con.utf8_to_gbk("，");
    string comma = "，";
    int pos;
    string fingerPrint;
    string temp = content_;
    while ((pos = temp.find(comma)) != string::npos)
    {
        try
        {
        	cout << pos << " "<< temp.size() <<endl;
            string sub;
            if (((pos-6)>=0)&& (temp.size()-pos)>3 &&(temp.size()>12) )
            {
                sub = temp.substr(pos - 6, 12);
            }
            else
            {
                break;
            }
            fingerPrint += sub;
            temp = temp.substr(pos + 6);
        }
        catch (const std::out_of_range &oor)
        {
            std::cerr << "Out of Range error: " << oor.what() << '\n';
        }
    }
    return fingerPrint;
}

int main(int argc, char const *argv[])
{
	//特征asd，特征asdf篇网页，比较top10词的交集，如果交集认为它们是互大于6个，
    string content = "交集认为它们是互大于6个，特征asd，特征asdf篇网页，比较top10词的交集，如果交集认为它们是互大于6个，于";

    // ifstream ifs("./utf8.txt");
    // ifs.seekg (0, ifs.end);
    // int length = ifs.tellg();
    // ifs.seekg (0, ifs.beg);
    // char *block = new char[length+1];
    // ifs.read(block,length);
    // block[length] = '\0';
    // string content(block);
    // delete[] block;
    // ifs.close();

    cout << extractFingerPrint(content);
    return 0;
}