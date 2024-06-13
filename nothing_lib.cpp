#include <vector>
#include "libdeflate.h"

unsigned char* libdefalte_compress(unsigned char *data, int data_len, int *out_len, int quality) {
    auto compressor = libdeflate_alloc_compressor(quality);
    std::vector<unsigned char> buffer(data_len);
    *out_len = libdeflate_zlib_compress(compressor, data, data_len, buffer.data(), buffer.size());
    libdeflate_free_compressor(compressor);
    auto outdata = (unsigned char*)malloc(*out_len);
    memcpy(outdata, buffer.data(), *out_len);
    return outdata;
}

#define STBIW_ZLIB_COMPRESS libdefalte_compress

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
