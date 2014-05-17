#ifndef RIPELIBTOOL_H_
#define RIPELIBTOOL_H_

#include <vector>
#include <string>

#include "Document.h"
#include "Configure.h"

class RipeLibTool
{
public:

	// std::vector<bool, std::string> &getFingerPrintVec(){
	// 	return fp_vec_;
	// }

	// std::vector<pair<int, int> > &getOffsetVec(){
	// 	return offset_vec_;
	// }

    RipeLibTool();

    ~RipeLibTool();

    void writeDistinctedLib2File();

private:
	void open_file(std::ifstream &ifs, const char *filename);
	void open_file(std::ofstream &ofs, const char *filename);
	void distinctSameDoc();
    void build_doc_offset_vec();
    void build_fingerPrint_vec();
    void save_fp_vec_to_file();
    void read_fingerPrint_file();
    void build_fp_offset_vec(const char* filename);
    void wirte_offsets_to_file(const char *filename);
    // vec < is_abandoned_,figerprint> the index is docId;
    std::vector<std::pair<bool,std::string> > fp_vec_;
    // read the doc's offset in this vec at constructor;
    std::vector<std::pair<int, int> > offset_vec_;

    // std::vector<Document> doc_vec_;
};

#endif