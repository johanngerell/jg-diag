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
        std::string xml;
        
        {
            tag svg{xml, "svg"};
            svg.attribute("width", m_width);
            svg.attribute("height", m_height);
            svg.attribute("version", "1.1");
            svg.attribute("baseProfile", "full");
            svg.attribute("xmlns", "http://www.w3.org/2000/svg");

            if (m_background)
            {
                tag background{svg, "rect"};
                background.attribute("width", "100%");
                background.attribute("height", "100%");
                background.attribute("fill", m_background_fill);
            }

            if (m_grid)
            {
                for (size_t x = m_grid_x; x <= m_width; x += m_grid_x)
                {
                    tag vertical{svg, "line"};
                    vertical.attribute("x1", x);
                    vertical.attribute("x2", x);
                    vertical.attribute("y1", 0);
                    vertical.attribute("y2", m_height);
                    vertical.attribute("stroke", m_grid_stroke);
                    vertical.attribute("stroke-width", 1);
                }

                for (size_t y = m_grid_y; y <= m_height; y += m_grid_y)
                {
                    tag horizontal{svg, "line"};
                    horizontal.attribute("x1", 0);
                    horizontal.attribute("x2", m_width);
                    horizontal.attribute("y1", y);
                    horizontal.attribute("y2", y);
                    horizontal.attribute("stroke", m_grid_stroke);
                    horizontal.attribute("stroke-width", 1);
                }
            }

            if (m_border)
            {
                tag border{svg, "rect"};
                border.attribute("x", 0);
                border.attribute("y", 0);
                border.attribute("width", m_width); 
                border.attribute("height", m_height);
                border.attribute("stroke", m_border_stroke);
                border.attribute("fill", "transparent");
                border.attribute("stroke-width", 1);
            }
        }

        return xml;
    }

private:
    class tag final
    {
    public:
        tag(std::string& xml, std::string_view name)
            : m_xml{xml}
            , m_name{name}
        {
            m_xml.append("<").append(m_name);
        }

        tag(tag& parent, std::string_view name)
            : m_xml{parent.m_xml}
            , m_name{name}
        {
            if (!parent.m_is_parent)
            {
                parent.m_is_parent = true;
                m_xml.append(">\n");
            }

            m_xml.append("<").append(name);
        }

        ~tag()
        {
            if (m_is_parent)
            {
                m_xml.append("</").append(m_name).append(">\n");
            }
            else
            {
                m_xml.append(" />\n");
            }
        }

        void attribute(std::string_view name, size_t value)
        {
            m_xml.append(1, ' ').append(name).append("=\"").append(std::to_string(value)).append("\"");
        }

        void attribute(std::string_view name, std::string_view value)
        {
            m_xml.append(1, ' ').append(name).append("=\"").append(value).append("\"");
        }
    
    private:
        std::string& m_xml;
        std::string_view m_name;
        bool m_is_parent{};
    };

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