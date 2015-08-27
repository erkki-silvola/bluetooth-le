
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sstream>
#include <assert.h>

#include "btle/log.h"

using namespace btle;

namespace {
    #define LOG_BUFFER_SIZE 1000
    static void default_adapter( const std::string& str )
    {
        std::printf("%s\n",str.c_str());
    }
}

log& log::instance()
{
    static log instance;
    return instance;
}

void log::set_options(int options, const std::string* file_path)
{
    options_ |= options;
    if( options_ & ::LOG_FILE_STREAM )
    {
        assert(file_path);
        ss_out_.open(file_path->c_str());
    }
}

/**
 * @brief log::install_adapter, you may optionally install different adapter
 *        if std::printf does not satisfy you
 * @param adapter
 */
void log::install_adapter(kAdapter adapter)
{
    adapter_ = adapter;
}

void log::trace(const char* tag,const char* method, const char* format, ...)
{
    if( options_ & LOG_PRINT || options_ & LOG_FILE_STREAM )
    {
        std::va_list ap;
        std::stringstream ss;
        std::string buf(LOG_BUFFER_SIZE, 0);
        va_start(ap, format);
        size_t size(vsnprintf(const_cast<char*>(buf.data()),LOG_BUFFER_SIZE,format,ap));
        buf.resize(size);
        va_end(ap);
        ss << '[' << tag << ']' << '[' << method << "] " << buf;
        if( options_ & LOG_PRINT )
            (*instance().adapter_)(ss.str());
        if( options_ & LOG_FILE_STREAM && ss_out_.is_open())
            ss_out_.write(ss.str().c_str(),ss.str().size());
    }
}

log::log()
: adapter_(&::default_adapter),
  options_(0)
{
}

// logfunc class
logfunc::logfunc( const char* func_name)
: func_name_(func_name)
{
    log::instance().trace("FUNC",func_name_,"[-->]");
}

logfunc::~logfunc()
{
    log::instance().trace("FUNC",func_name_,"[<--]");
}
