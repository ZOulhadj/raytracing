#include <iostream>
#include <fstream>
#include <chrono>

#include <glm.hpp>

struct ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

glm::vec3 ray_get_position(const ray& r, float t)
{
    const glm::vec3 position = r.direction * t;
    const glm::vec3 result = r.origin + position;

    return position;
}

double hit_sphere(const glm::vec3& center, float radius, const ray& r)
{
    const glm::vec3 sphere_position = r.origin - center;
    const float a = glm::dot(r.direction, r.direction);
    const float b = 2.0f * glm::dot(sphere_position, r.direction);
    const float c = glm::dot(sphere_position, sphere_position) - radius * radius;

    const float discriminant = b * b - 4 * a * c;

    if (discriminant < 0)
        return -1.0f;

    return (-b - glm::sqrt(discriminant)) / (2.0f * a);
}



glm::vec3 ray_calculate_pixel_color(const ray& r)
{
    float t = hit_sphere({ 0.0f, 0.0f, -1.0f }, 0.5f, r);
    if (t > 0.0f)
    {
        const glm::vec3 normal_direction = ray_get_position(r, t) - glm::vec3(0.0f, 0.0f, -1.0f);
        const glm::vec3 normal = glm::normalize(normal_direction);

        return 0.5f * (normal + 1.0f);
    }

    const glm::vec3 direction = glm::normalize(r.direction);
    t = 0.5 * (direction.y + 1.0);

    const glm::vec3 c1 = glm::vec3(1.0, 1.0, 1.0) * 1.0f - t;
    const glm::vec3 c2 = glm::vec3(0.5, 0.7, 1.0) * t;

    return c1 + c2;
}

int main(int argc, char** argv)
{
    std::ofstream image("image.ppm");
    if (!image.is_open())
    {
        std::cout << "Failed to write output image\n";
        return -1;
    }

    constexpr glm::ivec2 image_size = { 320, 180 };
    constexpr float aspect_ratio = static_cast<float>(image_size.x) / static_cast<float>(image_size.y);

    // z is the focal length of the camera/viewport
    constexpr glm::vec3 viewport_size = { 2.0f * aspect_ratio, 2.0f, 1.0f };

    constexpr glm::vec3 origin = glm::vec3(0.0f);

    constexpr glm::vec3 horizontal = glm::vec3(viewport_size.x, 0.0f, 0.0f);
    constexpr glm::vec3 vertical = glm::vec3(0.0f, viewport_size.y, 0.0f);
    constexpr glm::vec3 depth = glm::vec3(0.0f, 0.0f, viewport_size.z);

    constexpr glm::vec3 lower_left_corner = origin - horizontal / 2.0f - vertical / 2.0f - depth;


    const auto start_time = std::chrono::high_resolution_clock::now();


    image << "P3\n" << image_size.x << ' ' << image_size.y << "\n255\n";

    // Pixels are drawn from left to right (x), top to bottom (y) (device coordinates)
    for (int y = image_size.y - 1; y >= 0; --y)
    {
        for (int x = 0; x < image_size.x; ++x)
        {
            // -1 to 1 (normalized device coordinates) 
            //const float u = static_cast<float>(x) / image_width * 2.0 - 1.0;
            //const float v = static_cast<float>(y) / image_height * 2.0 - 1.0;
            glm::vec2 pixel_coord;
            pixel_coord.x = static_cast<float>(x) / (image_size.x - 1);
            pixel_coord.y = static_cast<float>(y) / (image_size.y - 1);

            // create a ray
            ray pixel_ray;
            pixel_ray.origin = origin;
            pixel_ray.direction = lower_left_corner + (pixel_coord.x * horizontal) + (pixel_coord.y * vertical) - origin;

            // r, g, b values are between 0 and 1
            const glm::vec3 pixel_color = ray_calculate_pixel_color(pixel_ray);

            // output pixel color
            const glm::vec3 output_color = pixel_color * 255.0f;
            image << output_color.x << ' ' << output_color.y << ' ' << output_color.z << '\n';
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const std::size_t duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Total time: " << duration << "ms\n";

    return 0;
}