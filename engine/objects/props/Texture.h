//
// Created by Ivan Ilin on 04/11/2023.
//

#ifndef RAYTRACE3D_TEXTURE_H
#define RAYTRACE3D_TEXTURE_H

#include <vector>

#include <io/Image.h>
#include <utils/FilePath.h>

class Texture {
private:
    // For resampling purposes we store resampled versions of the image
    // up until 1x1 image (avg color of the whole texture)
    std::vector<Image> _texture;

public:
    explicit Texture(const FilePath& filename);

    [[nodiscard]] Color get_pixel(uint16_t x, uint16_t y) const;
    [[nodiscard]] Color get_pixel_from_UV(const Vec2D& uv) const;

    // For resampling
    [[nodiscard]] Color get_pixel_from_UV(const Vec2D& uv, double area) const;

    [[nodiscard]] uint16_t width() const { return _texture.front().width(); }
    [[nodiscard]] uint16_t height() const { return _texture.front().height(); }
};


#endif //RAYTRACE3D_TEXTURE_H
