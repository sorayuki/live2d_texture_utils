#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <string>
#include <charconv>

std::vector<char> loadfile(std::filesystem::path path) {
    std::fstream fs(path, std::ios::in | std::ios::binary);
    if (!fs)
        return {};
    fs.seekg(0, std::ios::end);
    auto filesize = fs.tellg();
    fs.seekg(0, std::ios::beg);
    std::vector<char> buffer(filesize);
    fs.read(buffer.data(), filesize);
    return buffer;
}

using image_buffer = std::shared_ptr<stbi_uc>;

struct image_data {
    int width;
    int height;
    int channels;
    image_buffer image;

    void init(int width, int height, int channels) {
        this->width = width;
        this->height = height;
        this->channels = channels;
        this->image = image_buffer((stbi_uc*)malloc(width * height * channels), free);
    }

    image_data clone() {
        image_data ret = *this;
        auto image_size = width * height * channels;
        ret.image = image_buffer((stbi_uc*)malloc(image_size), free);
        memcpy(ret.image.get(), image.get(), image_size);
        return ret;
    }
};

std::optional<image_data> load_png(std::filesystem::path in_path) {
    auto in_buffer = loadfile(in_path);
    if (in_buffer.empty()) {
        return {};
    }

    int width, height;
    int channels;
    image_buffer rgba {
        stbi_load_from_memory((stbi_uc*)in_buffer.data(), in_buffer.size(), &width, &height, &channels, 4),
        stbi_image_free
    };

    if (!rgba.get())
        return {};

    return image_data{ width, height, channels, rgba };
}

bool save_png(std::filesystem::path out_path, const image_data& data) {
    std::fstream out_file(out_path, std::ios::out | std::ios::binary);
    auto writefunc = [](void* ctx, void* data, int size) {
        static_cast<std::fstream*>(ctx)->write((char*)data, size);
    };
    stbi_write_png_to_func(writefunc, &out_file, data.width, data.height, data.channels, data.image.get(), data.width * 4);
    return out_file.good();
}

image_data image_resize(const image_data& in_image, int dst_w, int dst_h) {
    image_data dst;
    dst.init(dst_w, dst_h, 4);
    stbir_resize(in_image.image.get(), in_image.width, in_image.height, in_image.width * in_image.channels,
        dst.image.get(), dst.width, dst.height, dst.width * dst.channels, 
        STBIR_RGBA, STBIR_TYPE_UINT8, STBIR_EDGE_REFLECT, STBIR_FILTER_DEFAULT
    );
    return dst;
}

int png_extract(int argc, wchar_t* argv[]) {
    std::filesystem::path in_path{argv[1]};
    auto out_rgb_path = std::filesystem::path(in_path).replace_extension(".rgb.png");
    auto out_a_path = std::filesystem::path(in_path).replace_extension(".a.png");

    auto in_data = load_png(in_path);
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

    save_png(out_rgb_path, out_rgb);
    save_png(out_a_path, out_a);

    return 0;
}


int png_resize(int argc, wchar_t* argv[]) {
    std::filesystem::path in_path{argv[1]};
    auto out_path = std::filesystem::path(in_path).replace_extension(".out.png");

    auto in_data = load_png(in_path);
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

    auto out_data = image_resize(*in_data, dst_w, dst_h);

    save_png(out_path, out_data);
    return 0;
}


int wmain(int argc, wchar_t* argv[]) {
    if (argc <= 2) {
        std::cerr << "usage: pngutil extract inputfile.png" << std::endl;
        std::cerr << "\tsplit png into separated rgb and alpha" << std::endl;
        std::cerr << "usage: pngutil resize inputfile.png 1024x1024" << std::endl;
        std::cerr << "\tresize png into lower resolution" << std::endl;
        return 1;
    }

    stbi_write_png_compression_level = 9;

    if (wcscmp(argv[1], L"extract") == 0 && argc == 3)
        return png_extract(argc - 1, argv + 1);
    else if (wcscmp(argv[1], L"resize") == 0 && argc == 4)
        return png_resize(argc - 1, argv + 1);
    else
        return 1;
}
