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

    std::string build()
    {
        auto attribute = [] (const char* name, const std::string& value)
        {
            return std::string(name).append("=\"").append(value).append("\"");
        };

        std::string svg;
        svg += "<svg " + attribute("width", std::to_string(m_width)) + " " + attribute("height", std::to_string(m_height)) + " version=\"1.1\" baseProfile=\"full\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        svg += "  <rect width=\"100%\" height=\"100%\" " + attribute("fill", m_fill) + " />\n";
        svg += "</svg>\n";

        return svg;
    }

private:
    size_t m_width{};
    size_t m_height{};
    std::string m_fill{"white"};
};

int main()
{
    svg_builder svg{400, 250, "grey"};

    std::cout << svg.build() << std::endl;
}