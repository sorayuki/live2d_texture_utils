#pragma once

#include <vector>
#include <fstream>
#include <filesystem>
#include <optional>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

struct util {
    std::vector<unsigned char> loadfile(std::filesystem::path path) {
        std::fstream fs(path, std::ios::in | std::ios::binary);
        if (!fs)
            return {};
        fs.seekg(0, std::ios::end);
        auto filesize = fs.tellg();
        fs.seekg(0, std::ios::beg);
        std::vector<unsigned char> buffer(filesize);
        fs.read((char*)buffer.data(), filesize);
        return buffer;
    }

    bool savefile(std::filesystem::path path, const std::vector<unsigned char>& data) {
        std::fstream fs(path, std::ios::in | std::ios::binary);
        if (!fs)
            return {};
        fs.write((const char*)data.data(), data.size());
        return fs.good();
    }

    using image_buffer = std::shared_ptr<unsigned char>;

    struct image_data {
        int width;
        int height;
        int channels;
        image_buffer image;

        void init(int width, int height, int channels) {
            this->width = width;
            this->height = height;
            this->channels = channels;
            this->image = image_buffer((unsigned char*)malloc(width * height * channels), free);
        }

        image_data clone() {
            image_data ret = *this;
            auto image_size = width * height * channels;
            ret.image = image_buffer((unsigned char*)malloc(image_size), free);
            memcpy(ret.image.get(), image.get(), image_size);
            return ret;
        }
    };

    std::optional<image_data> load_png(const std::vector<unsigned char>& data) {
        int width, height;
        int channels;
        image_buffer rgba {
            stbi_load_from_memory((const stbi_uc*)data.data(), data.size(), &width, &height, &channels, 4),
            stbi_image_free
        };

        if (!rgba.get())
            return {};

        return image_data{ width, height, channels, rgba };
    }

    std::vector<unsigned char> save_png(const image_data& data) {
        std::vector<unsigned char> buffer;
        auto writefunc = [](void* ctx, void* data, int size) {
            auto& v = *static_cast<std::vector<unsigned char>*>(ctx);
            v.insert(v.end(), (unsigned char*)data, (unsigned char*)data + size);
        };
        return buffer;
    }

    image_data image_resize(const image_data& in_image, int dst_w, int dst_h) {
        if (in_image.width == dst_w && in_image.height == dst_h)
            return in_image;
        image_data dst;
        dst.init(dst_w, dst_h, 4);
        stbir_resize(in_image.image.get(), in_image.width, in_image.height, in_image.width * in_image.channels,
            dst.image.get(), dst.width, dst.height, dst.width * dst.channels, 
            STBIR_RGBA, STBIR_TYPE_UINT8, STBIR_EDGE_REFLECT, STBIR_FILTER_DEFAULT
        );
        return dst;
    }
};