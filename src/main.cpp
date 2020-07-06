#include <iostream>
#include <string>

class xml_tag final
{
public:
    xml_tag(std::string& xml, std::string name)
        : m_xml{xml}
        , m_name{std::move(name)}
    {
        m_xml.append("<").append(m_name);
    }

    xml_tag(xml_tag& parent, std::string name)
        : m_xml{parent.m_xml}
        , m_name{std::move(name)}
    {
        if (!parent.m_is_parent)
        {
            parent.m_is_parent = true;
            m_xml.append(">\n");
        }

        m_xml.append("<").append(m_name);
    }

    ~xml_tag()
    {
        if (m_is_parent)
            m_xml.append("</").append(m_name).append(">\n");
        else
            m_xml.append(" />\n");
    }

    void attribute(std::string_view name, size_t value)
    {
        m_xml.append(1, ' ').append(name).append("=\"").append(std::to_string(value)).append("\"");
    }

    void attribute(std::string_view name, std::string_view value)
    {
        m_xml.append(1, ' ').append(name).append("=\"").append(value).append("\"");
    }

    void text(std::string_view text)
    {
        if (!m_is_parent)
        {
            m_is_parent = true;
            m_xml.append(">");
        }

        m_xml.append(text);
    }

private:
    std::string& m_xml;
    std::string m_name;
    bool m_is_parent{};
};

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
        m_background_fill = std::move(fill);
        return *this;
    }

    svg_builder& with_border(std::string stroke)
    {
        m_border = true;
        m_border_stroke = std::move(stroke);
        return *this;
    }

    svg_builder& with_grid(size_t x, size_t y, std::string stroke)
    {
        m_grid = true;
        m_grid_x = x;
        m_grid_y = y;
        m_grid_stroke = std::move(stroke);
        return *this;
    }
    
    std::string build()
    {
        std::string xml;
        
        {
            xml_tag svg{xml, "svg"};
            svg.attribute("width", m_width);
            svg.attribute("height", m_height);
            svg.attribute("version", "1.1");
            svg.attribute("baseProfile", "full");
            svg.attribute("xmlns", "http://www.w3.org/2000/svg");

            if (m_background)
            {
                xml_tag background{svg, "rect"};
                background.attribute("width", "100%");
                background.attribute("height", "100%");
                background.attribute("fill", m_background_fill);
            }

            if (m_grid)
            {
                for (size_t x = m_grid_x; x <= m_width; x += m_grid_x)
                {
                    xml_tag vertical{svg, "line"};
                    vertical.attribute("x1", x);
                    vertical.attribute("x2", x);
                    vertical.attribute("y1", 0);
                    vertical.attribute("y2", m_height);
                    vertical.attribute("stroke", m_grid_stroke);
                    vertical.attribute("stroke-width", 1);
                }

                for (size_t y = m_grid_y; y <= m_height; y += m_grid_y)
                {
                    xml_tag horizontal{svg, "line"};
                    horizontal.attribute("x1", 0);
                    horizontal.attribute("x2", m_width);
                    horizontal.attribute("y1", y);
                    horizontal.attribute("y2", y);
                    horizontal.attribute("stroke", m_grid_stroke);
                    horizontal.attribute("stroke-width", 1);
                }
            }

            {
                xml_tag frame1{svg, "rect"};
                frame1.attribute("x", 50);
                frame1.attribute("y", 50);
                frame1.attribute("width", 300); 
                frame1.attribute("height", 50);
                frame1.attribute("stroke", "black");
                frame1.attribute("fill", "transparent");
                frame1.attribute("stroke-width", 3);
            }

            {
                xml_tag text1{svg, "text"};
                text1.attribute("x", 75);
                text1.attribute("y", 75);
                text1.attribute("font-size", 25);
                text1.attribute("font-family", "sans-serif");
                text1.attribute("font-weight", "bold");
                text1.attribute("text-anchor", "left");
                text1.attribute("dominant-baseline", "middle");
                text1.attribute("stroke", "black");
                text1.attribute("fill", "white");
                text1.text("This is <tspan font-weight=\"bold\" fill=\"red\">bold and red</tspan>");
            }

            if (m_border)
            {
                xml_tag border{svg, "rect"};
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