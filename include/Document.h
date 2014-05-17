#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
// #include "MixSegment.hpp"
#include "Configure.h"

// class PriorityData{
// public:
//     std::string word_;
//     int frequency_;
//     friend bool operator<(const PriorityData &lo,const PriorityData &ro){
//         return lo.frequency_<ro.frequency_;
//     }
// };

class Document
{
public:
    Document();
    Document(int id);
    ~Document();

    void printSelf();

    void writeDoc2File(std::ofstream &ofs);

    void setId(int id)
    {
        docId_ = id;
    }
    void setUrl(std::string url)
    {
        url_ = url;
    }
    void setTitle(std::string title)
    {
        title_ = title;
    }
    void setContent(std::string content)
    {
        content_ = content;
    }
    
    std::string getFingerPrint(){
        return fingerPrint_;
    }

    // std::vector<std::string,int> &getTopTenWords(){
    //     return top_ten_;
    // }

private:

    void open_file(std::ifstream &ifs, const char *filename);
    void open_file(std::ofstream &ofs, const char *filename);

    void build_offset_vec(Configure *pconf);
    // 从文档字符串中解析出文档对象
    Document parseDoc2Obj(std::string &str);
    // 从ripelib中获取某篇文档的字符串
    std::string extractDocumentStringById(int id);
    // 从content中提取特征码
    std::string extractFingerPrint();

    //====================================

    // void buildTopTen(CppJieba::MixSegment &segementor);
    // // 1 . split words and counting word-frequency
    // void splitWords(std::map<std::string,int> &,CppJieba::MixSegment &);
    // // 2 . build big-root-heap by priority_queue
    // void saveIntoPriorityQueue(std::map<std::string,int> &,CppJieba::MixSegment &);

    int docId_;
    std::string url_;
    std::string title_;
    std::string content_;
    // to recognising a document
    std::string fingerPrint_;
    // word,frequency
    // std::vector<std::string,int> top_ten_;
};
#endif

