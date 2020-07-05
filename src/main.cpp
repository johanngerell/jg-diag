#include <iostream>
#include <string>

class svg_builder final
{
public:
    svg_builder() = default;

    svg_builder(size_t width, size_t height)
        : m_width{width}
        , m_height{height}
    {}

    svg_builder& with_background(std::string fill)
    {
        m_background = true;
        m_background_fill = fill;
        return *this;
    }

    svg_builder& with_border(std::string stroke)
    {
        m_border = true;
        m_border_stroke = stroke;
        return *this;
    }

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
        {
            tag svg{m_svg, "svg"};
            attribute("width", m_width);
            attribute("height", m_height);
            attribute("version", "1.1");
            attribute("baseProfile", "full");
            attribute("xmlns", "http://www.w3.org/2000/svg");
            svg.end();

            if (m_background)
            {
                self_closed_tag background{m_svg, "rect"};
                attribute("width", "100%");
                attribute("height", "100%");
                attribute("fill", m_background_fill);
            }

            if (m_grid)
            {
                for (size_t x = m_grid_x; x <= m_width; x += m_grid_x)
                {
                    self_closed_tag vertical{m_svg, "line"};
                    attribute("x1", x);
                    attribute("x2", x);
                    attribute("y1", 0);
                    attribute("y2", m_height);
                    attribute("stroke", m_grid_stroke);
                    attribute("stroke-width", 1);
                }

                for (size_t y = m_grid_y; y <= m_height; y += m_grid_y)
                {
                    self_closed_tag horizontal{m_svg, "line"};
                    attribute("x1", 0);
                    attribute("x2", m_width);
                    attribute("y1", y);
                    attribute("y2", y);
                    attribute("stroke", m_grid_stroke);
                    attribute("stroke-width", 1);
                }
            }

            if (m_border)
            {
                self_closed_tag border{m_svg, "rect"};
                attribute("x", 0);
                attribute("y", 0);
                attribute("width", m_width); 
                attribute("height", m_height);
                attribute("stroke", m_border_stroke);
                attribute("fill", "transparent");
                attribute("stroke-width", 1);
            }
        }
        
        return m_svg;
    }

private:
    void attribute(const char* name, size_t value)
    {
        m_svg.append(1, ' ').append(name).append("=\"").append(std::to_string(value)).append("\"");
    }

    void attribute(const char* name, std::string_view value)
    {
        m_svg.append(1, ' ').append(name).append("=\"").append(value).append("\"");
    }

    struct self_closed_tag final
    {
        std::string& m_xml;

        self_closed_tag(std::string& xml, std::string_view name)
            : m_xml{xml}
        {
            m_xml.append("<").append(name);
        }

        ~self_closed_tag()
        {
            m_xml.append(" />\n");
        }
    };

    struct tag final
    {
        std::string& m_xml;
        std::string_view m_name;

        tag(std::string& xml, std::string_view name)
            : m_xml{xml}
            , m_name{name}
        {
            m_xml.append("<").append(m_name);
        }

        ~tag()
        {
            m_xml.append("</").append(m_name).append(">\n");
        }

        void end()
        {
            m_xml.append(">\n");
        }
    };

    std::string m_svg;
    size_t m_width{};
    size_t m_height{};
    bool m_background{};
    std::string m_background_fill;
    bool m_border{};
    std::string m_border_stroke;
    bool m_grid{};
    size_t m_grid_x{};
    size_t m_grid_y{};
    std::string m_grid_stroke;
};

int main()
{
    svg_builder svg{1024, 768};
    svg.with_background("white");
    svg.with_border("black");
    svg.with_grid(50, 50, "#eeeeee");

    std::cout << svg.build() << std::endl;
}