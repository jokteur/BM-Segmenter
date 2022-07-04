#include <string>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <zlib.h>

namespace Data {
    /** Compress a STL string using zlib with given compression level and return
      * the binary data. */
    std::string compress_string(const std::string& str,
        int compressionlevel = Z_BEST_COMPRESSION);


    /** Decompress an STL string using zlib and return the original data. */
    std::string decompress_string(const std::string& str);
}