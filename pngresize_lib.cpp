#include "utils.h"

#include <windows.h>

typedef void (__cdecl *on_finish_callback)(int code, const void* data, size_t size);

class PngResize: public util {
public:
    void Run(const unsigned char* in_data, size_t in_size, int width, int height, on_finish_callback callback) {
        std::vector<unsigned char> in_buf(in_data, in_data + in_size);
        auto in_img = load_png(in_buf);
        if (!in_img) {
            callback(-1, nullptr, 0);
            return;
        }
        auto out_data = save_png(image_resize(*in_img, width, height));
        callback(0, out_data.data(), out_data.size());
    }
};

extern "C"
__declspec(dllexport)
void __cdecl ResizePNG(const unsigned char* in_data, size_t in_size, int width, int height, on_finish_callback callback) {
    PngResize{}.Run(in_data, in_size, width, height, callback);
}
