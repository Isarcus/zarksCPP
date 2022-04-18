#include <iostream>

namespace zmath
{
    // StreamKeeper is a class that stores a reference to an input
    // stream object, and upon going out of scope, restores the
    // internal position of that stream object to be whatever it
    // was when the StreamKeeper constructor was called.
    class StreamKeeper
    {
    public:
        StreamKeeper(std::istream& is);
        ~StreamKeeper();

        // @return The current stream offset relative to when this
        //         StreamKeeper's constructor was called
        std::streamoff Offset() const;

    private:
        std::istream& is;
        const std::streampos startpos;
    };
}
