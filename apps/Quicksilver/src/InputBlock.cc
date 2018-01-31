#include "InputBlock.hh"
#ifdef HAVE_CALIPER
#include<caliper/cali.h>
#endif
#include <sstream>
#include <cstring>

using std::map;
using std::ostringstream;
using std::string;


InputBlock::InputBlock(const string& blockName)
    : _blockName(blockName)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
}

void InputBlock::addPair(const string& keyword, const string& value)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    _kvPair[keyword] = value;
}

void InputBlock::serialize(std::vector<char>& buf) const
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    ostringstream out;
    out << _blockName << '\0';
    for (auto iter=_kvPair.begin(); iter!=_kvPair.end(); ++iter)
        out << iter->first << '\0' << iter->second <<'\0';
    string tmp = out.str();
    buf.clear();
    buf.insert(buf.begin(), tmp.begin(), tmp.end());
}

void InputBlock::deserialize(const std::vector<char>& buf)
{
#ifdef HAVE_CALIPER
CALI_CXX_MARK_FUNCTION;
#endif
    const char* tmp = &buf[0];
    const char* end = tmp + buf.size();

    _blockName = tmp;
    tmp += strlen(tmp) +1;

    while (tmp < end)
    {
        const char* keyword = tmp;
        tmp += strlen(tmp) +1;
        const char* value = tmp;
        tmp += strlen(tmp) +1;
        _kvPair[keyword] = value;
    }
}
