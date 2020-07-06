#include <iostream>
#include <string>

class xml_tag final
{
public:
    static xml_tag root(std::string& xml, std::string name)
    {
        return {xml, name};
    }

    static xml_tag child(xml_tag& parent, std::string name)
    {
        return {parent, name};
    }

    xml_tag(xml_tag&& other)
        : m_xml{other.m_xml}
        , m_name{std::move(other.m_name)}
        , m_is_parent{other.m_is_parent}
    {
        other.m_xml = nullptr;
        other.m_is_parent = false;
    }

    ~xml_tag()
    {
        if (m_xml)
        {
            if (m_is_parent)
                m_xml->append("</").append(m_name).append(">");
            else
                m_xml->append(" />");
            
            if (!m_is_inline)
                m_xml->append("\n");
        }
    }

    xml_tag& attribute(std::string_view name, size_t value)
    {
        m_xml->append(1, ' ').append(name).append("=\"").append(std::to_string(value)).append("\"");
        return *this;
    }

    xml_tag& attribute(std::string_view name, std::string_view value)
    {
        m_xml->append(1, ' ').append(name).append("=\"").append(value).append("\"");
        return *this;
    }

    xml_tag& text(std::string_view text)
    {
        if (!m_is_parent)
        {
            m_is_parent = true;
            m_xml->append(">");
        }

        m_xml->append(text);
        return *this;
    }

    void no_newline()
    {
        m_is_inline = true;
    }

private:
    xml_tag(std::string& xml, std::string name)
        : m_xml{&xml}
        , m_name{std::move(name)}
    {
        m_xml->append("<").append(m_name);
    }

    xml_tag(xml_tag& parent, std::string name)
        : m_xml{parent.m_xml}
        , m_name{std::move(name)}
    {
        if (!parent.m_is_parent)
        {
            parent.m_is_parent = true;

            if (!parent.m_is_inline)
                m_xml->append(">\n");
        }

        m_xml->append("<").append(m_name);
    }

    std::string* m_xml;
    std::string m_name;
    bool m_is_parent{};
    bool m_is_inline{};
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
            auto svg = xml_tag::root(xml, "svg");
            svg.attribute("width", m_width);
            svg.attribute("height", m_height);
            svg.attribute("version", "1.1");
            svg.attribute("baseProfile", "full");
            svg.attribute("xmlns", "http://www.w3.org/2000/svg");

            if (m_background)
                xml_tag::child(svg, "rect")
                    .attribute("width", "100%")
                    .attribute("height", "100%")
                    .attribute("fill", m_background_fill);

            if (m_grid)
            {
                for (size_t x = m_grid_x; x <= m_width; x += m_grid_x)
                    xml_tag::child(svg, "line")
                        .attribute("x1", x)
                        .attribute("x2", x)
                        .attribute("y1", 0)
                        .attribute("y2", m_height)
                        .attribute("stroke", m_grid_stroke)
                        .attribute("stroke-width", 1);

                for (size_t y = m_grid_y; y <= m_height; y += m_grid_y)
                    xml_tag::child(svg, "line")
                        .attribute("x1", 0)
                        .attribute("x2", m_width)
                        .attribute("y1", y)
                        .attribute("y2", y)
                        .attribute("stroke", m_grid_stroke)
                        .attribute("stroke-width", 1);
            }

            xml_tag::child(svg, "rect")
                .attribute("x", 50)
                .attribute("y", 50)
                .attribute("width", 300)
                .attribute("height", 50)
                .attribute("stroke", "black")
                .attribute("fill", "transparent")
                .attribute("stroke-width", 3);

            std::string s{"This is "};
            xml_tag::root(s, "tspan")
                .attribute("font-weight", "bold")
                .attribute("fill", "red")
                .text("bold and red")
                .no_newline();

            xml_tag::child(svg, "text")
                .attribute("x", 75)
                .attribute("y", 75)
                .attribute("font-size", 25)
                .attribute("font-family", "sans-serif")
                .attribute("font-weight", "bold")
                .attribute("text-anchor", "left")
                .attribute("dominant-baseline", "middle")
                .attribute("stroke", "black")
                .attribute("fill", "white")
                .text(s);

            if (m_border)
                xml_tag::child(svg, "rect")
                    .attribute("x", 0)
                    .attribute("y", 0)
                    .attribute("width", m_width)
                    .attribute("height", m_height)
                    .attribute("stroke", m_border_stroke)
                    .attribute("fill", "transparent")
                    .attribute("stroke-width", 1);
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