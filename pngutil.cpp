#include "utils.h"

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <string>
#include <charconv>

class png_extract: public util {
public:
    int run(int argc, wchar_t* argv[]) {
        std::filesystem::path in_path{argv[1]};
        auto out_rgb_path = std::filesystem::path(in_path).replace_extension(".rgb.png");
        auto out_a_path = std::filesystem::path(in_path).replace_extension(".a.png");

        auto in_data = load_png(loadfile(in_path));
        if (!in_data) {
            std::cerr << "fail to read input file" << std::endl;
            return 1;
        }

        if (in_data->channels != 4) {
            std::cerr << "Not RGBA input png." << std::endl;
            return 1;
        }

        auto in_rgba_pixels = in_data->width * in_data->height;
        auto in_rgba_size = in_rgba_pixels * in_data->channels;

        auto out_rgb = in_data->clone();
        auto out_a = in_data->clone();

        auto out_rgb_ptr = out_rgb.image.get();
        auto out_a_ptr = out_a.image.get();

        for(int i = 0; i < in_rgba_pixels; ++i) {
            out_rgb_ptr[4 * i + 3] = 0xff;
            out_a_ptr[4 * i + 0] = out_a_ptr[4 * i + 1] = out_a_ptr[4 * i + 2] = out_a_ptr[4 * i + 3];
            out_a_ptr[4 * i + 3] = 0xff;
        }

        auto writefunc = [](void* ctx, void* data, int size) {
            static_cast<std::fstream*>(ctx)->write((char*)data, size);
        };

        savefile(out_rgb_path, save_png(out_rgb));
        savefile(out_a_path, save_png(out_a));

        return 0;
    }
};


class png_resize: public util {
public:
    int run(int argc, wchar_t* argv[]) {
        std::filesystem::path in_path{argv[1]};
        auto out_path = std::filesystem::path(in_path).replace_extension(".out.png");

        auto in_data = load_png(loadfile(in_path));
        if (!in_data) {
            std::cerr << "fail to read input file" << std::endl;
            return 1;
        }

        if (in_data->channels != 4) {
            std::cerr << "Not RGBA input png." << std::endl;
            return 1;
        }

        int dst_w, dst_h;
        if (swscanf(argv[2], L"%dx%d", &dst_w, &dst_h) != 2) {
            std::cerr << "incorrect target resolution" << std::endl;
            return 1;
        }

        while(in_data->width > dst_w && in_data->height > dst_h) {
            auto cur_dst_w = in_data->width / 2;
            auto cur_dst_h = in_data->height / 2;
            if (cur_dst_w < dst_w)
                cur_dst_w = dst_w;
            if (cur_dst_h < dst_h)
                cur_dst_h = dst_h;
            std::cerr << "resizing to " << cur_dst_w << "x" << cur_dst_h << std::endl;
            in_data = image_resize(*in_data, cur_dst_w, cur_dst_h);
        }

        savefile(out_path, save_png(*in_data));
        return 0;
    }
};


int wmain(int argc, wchar_t* argv[]) {
    if (argc <= 2) {
        std::cerr << "usage: pngutil extract inputfile.png" << std::endl;
        std::cerr << "\tsplit png into separated rgb and alpha" << std::endl;
        std::cerr << "usage: pngutil resize inputfile.png 1024x1024" << std::endl;
        std::cerr << "\tresize png into lower resolution" << std::endl;
        return 1;
    }

    stbi_write_png_compression_level = 12;

    if (wcscmp(argv[1], L"extract") == 0 && argc == 3)
        return png_extract{}.run(argc - 1, argv + 1);
    else if (wcscmp(argv[1], L"resize") == 0 && argc == 4)
        return png_resize{}.run(argc - 1, argv + 1);
    else
        return 1;
}
