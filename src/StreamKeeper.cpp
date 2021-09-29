#include <zarks/io/StreamKeeper.h>

namespace zmath
{

StreamKeeper::StreamKeeper(std::istream& is)
    : is(is)
    , startpos(is.tellg())
{}

StreamKeeper::~StreamKeeper()
{
    is.seekg(startpos);
}

std::istream::streamoff StreamKeeper::Offset() const
{
    return is.tellg() - startpos;
}

} // namespace zmath