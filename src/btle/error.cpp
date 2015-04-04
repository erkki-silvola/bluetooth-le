
#include "btle/error.h"
#include "btle/utility.h"

using namespace btle;

error::error()
: base()
{
}

error::error(const error& other)
: base(other)
{
}

error::error(int code)
: base(utility::to_string(code),
       "Error code: " + utility::to_string(code)),
  code_(code)
{
}

error::error(int code,const std::string& description)
: base(utility::to_string(code),description),
  code_(code)
{
}

int error::code() const
{
    return code_;
}

error& error::operator = (const error& other)
{
    code_ = other.code();
    description_ = other.description();
    return *this;
}

bool error::operator == (const error& other) const
{
    return this->code_ == other.code();
}


