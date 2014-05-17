#include "Document.h"
#include "RipeLibTool.h"
#include "Configure.h"
#include "Lcs.h"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>


using namespace std;


// save fingerprint index lib offset
vector<pair<int, int> > fp_offsets;

void RipeLibTool::open_file(std::ifstream &ifs, const char *filename)
{
    ifs.close();
    ifs.clear();
    ifs.open(filename);
    if (!(ifs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}

void RipeLibTool::open_file(std::ofstream &ofs, const char *filename)
{
    ofs.close();
    ofs.clear();
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw std::runtime_error("open file ");
    }
}

void RipeLibTool::build_doc_offset_vec()
{
    Configure *pconf = Configure::getInstance();
    string offset_path = pconf->getConfigByName("offset_path");
    ifstream libfile;
    open_file(libfile, offset_path.c_str());
    string line;
    while (getline(libfile, line))
    {
        istringstream sl(line);
        int id, offset, length;
        sl >> id >> offset >> length;
        offset_vec_.push_back(make_pair(offset, length));
    }
    libfile.close();
}

void RipeLibTool::build_fingerPrint_vec()
{
    int docNum = offset_vec_.size();
    for (int ix = 1; ix <= docNum ; ++ix)
    {
        Document doc(ix);
        fp_vec_.push_back(make_pair(false, doc.getFingerPrint()));
        // doc_vec_.push_back(doc);
    }
}
int abondoned_size0 = 0;
int abondoned = 0;
void RipeLibTool::distinctSameDoc()
{
    for (std::vector<pair<bool, std::string> >::size_type ix = 0; ix != fp_vec_.size() - 1; ++ix)
    {

#ifndef NDEBUG
        cout << " ix is : " << ix << endl;
#endif
        string sa = fp_vec_[ix].second;
        if (sa.size() == 1)
        {
            cout << " doc " << ix << " is abondoned !" << endl;
            fp_vec_[ix].first = true;
            ++abondoned_size0;
            continue;
        }
        for (std::vector<pair<bool, std::string> >::size_type jx = ix + 1; (fp_vec_[ix].first == false) &&  (fp_vec_[jx].first == false) && jx != fp_vec_.size(); ++jx)
        {

            //对于每两篇网页，计算特征字符串S1和特征字符串S2之间的最长公共子序列LCS
            string sb = fp_vec_[jx].second;
            if (sb.size() == 1)
            {
                cout << " doc " << jx << " is abondoned !" << endl;
                fp_vec_[jx].first = true;
                ++abondoned_size0;
                continue;
            }
            float lcs = getLongestCommonSubsequenceLength(sa, sb);
            //如果len(LCS)/min(len(S1), len(S2))>0.6, 则认为重复,淘汰短的那篇
            float min = sa.size() < sb.size() ? sa.size() : sb.size();
            float res = (lcs / min);
#ifndef NDEBUG
            // cout << "res : ---  " << res << "   ----sa.size()"<<sa.size()<<"sb.size()"<<sb.size()<<"lcs"<<lcs <<endl;
#endif
            if ( res > 0.3)
            {
                if (min == sa.size())
                {
                    cout << " doc " << ix << " is abondoned ! because len(LCS)/min(len(S1), len(S2))>0.6" << endl;
                    ++abondoned;
                    fp_vec_[ix].first = true;
                }
                else
                {
                    cout << " doc " << jx << " is abondoned ! because len(LCS)/min(len(S1), len(S2))>0.6" << endl;
                    ++abondoned;
                    fp_vec_[jx].first = true;
                }
            }
        }
    }
}



void RipeLibTool::wirte_offsets_to_file(const char *filename)
{
    int docid = 0;
    ofstream ofs;
    ofs.open(filename);
    if (!(ofs.is_open()))
    {
        throw runtime_error("open offset file");
    }
    for (vector<pair<int, int> >::iterator iter = fp_offsets.begin();
            iter != fp_offsets.end(); ++iter)
    {
        ofs << ++docid << '\t' << (*iter).first << '\t' << (*iter).second << "\n";
    }
    ofs.close();
}

void RipeLibTool::save_fp_vec_to_file()
{
    Configure *pconf = Configure::getInstance();
    string fp_path = pconf->getConfigByName("fp_path");
    string fp_index_path = pconf->getConfigByName("fp_index_path");
    ofstream ofs;
    open_file(ofs, fp_path.c_str());
    for (auto fp : fp_vec_)
    {
        int offset = ofs.tellp();
        ofs << fp.second << '\n';
        int length = ofs.tellp() - offset;
        fp_offsets.push_back(make_pair(offset, length));
    }
    wirte_offsets_to_file(fp_index_path.c_str());
    ofs.close();
}

void RipeLibTool::read_fingerPrint_file()
{
    Configure *pconf = Configure::getInstance();
    // finger print lib path
    string fp_path = pconf->getConfigByName("fp_path");
    // finger print_lib index_lib path
    string fp_index_path = pconf->getConfigByName("fp_index_path");
    // prepair the index lib
    build_fp_offset_vec(fp_index_path.c_str());

    int docNum = fp_offsets.size();

    ifstream ifs;
    open_file(ifs, fp_path.c_str());
    ifs.seekg(0, ifs.beg);

    for (int id = 1; id < docNum ; ++id)
    {
        //get offsets and length
        int offset = fp_offsets[id - 1].first;
        int length = fp_offsets[id - 1].second;
        //jump to offset
        ifs.seekg(offset, ifs.beg);
        //read fp in block
        char *block = new char[length + 1];
        ifs.read(block, length);
        block[length] = 0;
        string fingerprint(block);
        fp_vec_.push_back(make_pair(false, fingerprint));
        delete[] block;
    }

    ifs.close();
}



void RipeLibTool::writeDistinctedLib2File()
{
    Configure *pconf = Configure::getInstance();
    string lib_path = pconf->getConfigByName("new_lib_path");
    ofstream ofs(lib_path.c_str(), ofstream::out | ofstream::app);
    if (!(ofs.is_open()))
    {
        throw runtime_error("open lib_path failed");
    }
    // traverse the fp_vec_,select the non abondoned one
    for (std::vector<std::pair<bool, std::string> >::size_type ix = 0; ix != fp_vec_.size(); ++ix)
    {
        if (fp_vec_[ix].first == false)
        {
            Document doc(ix);

            doc.writeDoc2File(ofs);
        }
    }
    ofs.close();
}

void RipeLibTool::build_fp_offset_vec(const char *filename)
{
    ifstream libfile;
    open_file(libfile, filename);
    string line;
    while (getline(libfile, line))
    {
        istringstream sl(line);
        int id, offset, length;
        sl >> id >> offset >> length;
        fp_offsets.push_back(make_pair(offset, length));
    }
    libfile.close();
}

RipeLibTool::RipeLibTool(): fp_vec_(), offset_vec_()
{
    cout << "build_offset_vec..." << endl;
    build_doc_offset_vec();
#ifdef BUILD_FP_VEC
    cout << "build_fingerPrint_vec..." << endl;
    build_fingerPrint_vec();
    cout << "save_fp_vec_to_file ..." << endl;
    save_fp_vec_to_file();
#endif
#ifndef BUILD_FP_VEC
    cout << "reading fp_vec_ to file..." << endl;
    read_fingerPrint_file();
#endif
    cout << "distinctSameDoc..." << endl;
    distinctSameDoc();
    cout << "abondoned " << abondoned_size0 << "+" << abondoned << "docs" << endl;
    cin >> abondoned;
}

RipeLibTool::~RipeLibTool()
{

}