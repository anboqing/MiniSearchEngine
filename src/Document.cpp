#include "Document.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>

#include "EncodingConverter.h"


// using namespace CppJieba;

using namespace std;


std::vector<std::pair<int, int> > offset_vec;

void Document::open_file(std::ifstream &ifs, const char *filename)
{
    ifs.close();
    ifs.clear();
    ifs.open(filename);
    if (!(ifs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}

void Document::open_file(std::ofstream &ofs, const char *filename)
{
    ofs.close();
    ofs.clear();
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}

void Document::build_offset_vec(Configure *pconf)
{
    string offset_path = pconf->getConfigByName("offset_path");
    ifstream libfile;
    open_file(libfile, offset_path.c_str());
    string line;
    while (getline(libfile, line))
    {
        istringstream sl(line);
        int id, offset, length;
        sl >> id >> offset >> length;
        offset_vec.push_back(make_pair(offset, length));
    }
    libfile.close();
}

// 从ripelib中获取某篇文档的字符串
string Document::extractDocumentStringById(int id)
{
#ifndef NDEBUG
    cout << "extractDocumentStringById.." << id << endl;
#endif
    Configure *pconf = Configure::getInstance();
    string lib_path = pconf->getConfigByName("lib_path");
    // build offset_vec
    build_offset_vec(pconf);

    ifstream in;
    open_file(in, lib_path.c_str());
    // get offset and length
    in.seekg(0, in.beg);
    int offset = offset_vec[id - 1].first;
    int length = offset_vec[id - 1].second;
    // jump to offset
    in.seekg(offset, in.beg);
    // read doc in block;
    char *block = new char[length + 1];
    //This function simply copies a block of data,
    //!!! without checking its contents nor appending a null character at the end.
    in.read(block, length);
    // so append NULL charactor at the end;
    block[length] = 0;

    string document(block);
    in.close();
    delete[] block;
    return document;
}

// 从文档字符串中解析出文档对象
Document Document::parseDoc2Obj(std::string &str)
{
#ifndef NDEBUG
    cout << "parseDoc2Obj" << endl;
#endif
    Document doc;
    // get docid;
    int begin = str.find("<docid>");
    int end = str.find("</docid>");
    if (begin != std::string::npos && end != std::string::npos)
    {
        // found docid
        string sid(str, begin + 7, (end - begin - 7));
        // std::stoi only support by c++11
        int id = stoi(sid);
        doc.setId(id);
    }
    // get title
    begin = str.find("<title>");
    end = str.find("</title>");
    if (begin != std::string::npos && end != std::string::npos)
    {
        string title(str, begin + 7, (end - begin - 7));
        doc.setTitle(title);
    }
    // get url
    begin = str.find("<url>");
    end = str.find("</url>");
    if (begin != std::string::npos && end != std::string::npos)
    {
        string url(str, begin + 5, (end - begin - 5));
        doc.setUrl(url);
    }
    // get content
    begin = str.find("<content>");
    end = str.find("</content>");
    if (begin != std::string::npos && end != std::string::npos)
    {
        string content(str, begin + 9, (end - begin - 9));
        doc.setContent(content);
    }

    return doc;
}
/*
    从content中提取特征码
    提取网页中每个逗号前后的5个字节，
    这样每个网页可以提取出若干的字符串，
    将这些字符串顺序链接形成一个特征字符串S
*/
string Document::extractFingerPrint()
{
#ifndef NDEBUG
    cout << "extractFingerPrint" << endl;
#endif
    EncodingConverter con;
    // 别忘了还有UTF8逗号啊！！！！！！不算UTF8逗号很多文章都没有指纹了！！
    string comma = con.utf8_to_gbk("，");
    int pos;//这里千万不能用size_t要不然搞死你都不知到outofrange从哪冒出来的！！
    string fingerPrint;
    string temp = content_;
    while (((pos = temp.find(comma)) != string::npos) || ((pos = temp.find(",")) != string::npos))
    {
        try
        {
            string sub;
            if (((pos - 6) >= 0) && (temp.size() - pos) > 3 && (temp.size() > 12) )
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

void Document::printSelf()
{
    cout << "id : " << docId_ << endl;
    cout << "title : " << title_ << endl;
    cout << "url :" << url_ << endl;
    // cout << "content" << content_ << endl;
    cout << "fingerPrint_" << fingerPrint_ << endl;
}

void Document::writeDoc2File(std::ofstream &ofs)
{
    ofs << "<doc>" << endl;
    ofs << '\t' << "<docid>" << docId_ << "</docid>" << endl;
    ofs << '\t' << "<url>" << url_ << "</url>" << endl;
    ofs << '\t' << "<title>" << title_ << "</title>" << endl;
    ofs << '\t' << "<content>" << content_ << "</content>" << endl;
    ofs << "</doc>" << endl << endl;
}

Document::Document(int id):
    docId_(id), url_(), title_(),
    content_(), fingerPrint_()
{
    string docstr = extractDocumentStringById(docId_);
    *this = parseDoc2Obj(docstr);
#ifdef BUILD_FP_VEC
    fingerPrint_ = extractFingerPrint();
#endif
#ifdef BUILD_TOP_TEN

#endif
}

Document::Document():
    docId_(0), url_(), title_(),
    content_(), fingerPrint_()
{
}

Document::~Document()
{

}


// void Document::buildTopTen(MixSegment &segementor)
// {

//     map<string, int> frequency_mp;
//     // 1 . split words and counting word-frequency
//     splitWords(frequency_mp, segementor);




// }

// // 1 . split words and counting word-frequency
// void Document::splitWords(std::map<std::string, int> &mp, CppJieba::MixSegment &segementor)
// {
//     vector<string> words;
//     segementor.cut(content_, words);
//     countingWordFrequency(words, mp);

//     saveIntoPriorityQueue(mp, segementor);

// }
// // 2 . counting word frequency
// void countingWordFrequency(vector<string> &words, std::map<std::string, int> &mp)
// {
//     for (auto word : words)
//     {
//         ++mp[word];
//     }
// }

// // 3 . build big-root-heap by priority_queue
// void Document::saveIntoPriorityQueue(std::map<std::string, int> &mp, MixSegment &segementor)
// {
//     priority_queue<PriorityData> pqueue;
//     PriorityData data;
//     for (auto pr : mp)
//     {
//         data.frequency_ = pr.second;
//         data.word_ = pr.first;
//         pqueue.push(data);
//     }
// }

/*
    Configure *pconf = Configure::getInstance();
    string dict_path = pconf->getConfigByName("dict_path");
    string model_path = pconf->getConfigByName("model_path");

    MixSegment segementor(dict_path.c_str(), model_path.c_str());

*/