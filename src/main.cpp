#include <iostream>
#include <string>

class svg_builder final
{
public:
    svg_builder() = default;

    svg_builder(size_t width, size_t height, std::string fill)
        : m_width{width}
        , m_height{height}
        , m_fill{std::move(fill)}
    {}

    svg_builder& with_grid(size_t x, size_t y, std::string stroke)
    {
        m_grid = true;
        m_grid_x = x;
        m_grid_y = y;
        m_grid_stroke = stroke;
        return *this;
    }
    
    std::string build()
    {
        auto attribute_i = [] (const char* name, size_t value)
        {
            return std::string(name).append("=\"").append(std::to_string(value)).append("\"");
        };

        auto attribute_s = [] (const char* name, std::string_view value)
        {
            return std::string(name).append("=\"").append(value).append("\"");
        };

        std::string svg;
        svg += "<svg " +
               attribute_i("width", m_width) + " " +
               attribute_i("height", m_height) + " " +
               attribute_s("version", "1.1") + " " +
               attribute_s("baseProfile", "full") + " " +
               attribute_s("xmlns", "http://www.w3.org/2000/svg") +
               ">\n";

        svg += "<rect " +
               attribute_s("width", "100%") + " " +
               attribute_s("height", "100%") + " " +
               attribute_s("fill", m_fill) +
               " />\n";

        if (m_grid)
        {
            for (size_t x = m_grid_x; x <= m_width; x += m_grid_x)
            {
                svg += "<line " +
                       attribute_i("x1", x) + " " +
                       attribute_i("x2", x) + " " +
                       attribute_i("y1", 0) + " " +
                       attribute_i("y2", m_height) + " " +
                       attribute_s("stroke", m_grid_stroke) + " " +
                       attribute_i("stroke-width", 1) +
                       " />\n";
            }

            for (size_t y = m_grid_y; y <= m_height; y += m_grid_y)
            {
                svg += "<line " +
                       attribute_i("x1", 0) + " " +
                       attribute_i("x2", m_width) + " " +
                       attribute_i("y1", y) + " " +
                       attribute_i("y2", y) + " " +
                       attribute_s("stroke", m_grid_stroke) + " " +
                       attribute_i("stroke-width", 1) +
                       " />\n";
            }
        }

        svg += "</svg>\n";

        return svg;
    }

private:
    size_t m_width{};
    size_t m_height{};
    std::string m_fill{"white"};
    bool m_grid{};
    size_t m_grid_x{};
    size_t m_grid_y{};
    std::string m_grid_stroke{"lightgrey"};
};

int main()
{
    svg_builder svg{400, 250, "white"};
    svg.with_grid(50, 50, "#eeeeee");

    std::cout << svg.build() << std::endl;
}