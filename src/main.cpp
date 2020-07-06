#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

class xml_tag final
{
public:
    xml_tag(std::ostream& stream, std::string name)
        : m_stream{&stream}
        , m_name{std::move(name)}
    {
        *m_stream << "<" << m_name;
    }

    xml_tag(xml_tag& parent, std::string name)
        : m_stream{parent.m_stream}
        , m_name{std::move(name)}
    {
        if (!parent.m_is_parent)
        {
            parent.m_is_parent = true;

            if (!parent.m_is_inline)
                *m_stream << ">\n";
        }

        *m_stream << "<" << m_name;
    }

    xml_tag(xml_tag&& other)
        : m_stream{other.m_stream}
        , m_name{std::move(other.m_name)}
        , m_is_parent{other.m_is_parent}
    {
        other.m_stream = nullptr;
        other.m_is_parent = false;
    }

    ~xml_tag()
    {
        if (m_stream)
        {
            if (m_is_parent)
                *m_stream << "</" << m_name << ">";
            else
                *m_stream << " />";
            
            if (!m_is_inline)
                *m_stream << "\n";
        }
    }

    xml_tag& attribute(std::string_view name, size_t value)
    {
        *m_stream << ' ' << name << "=\"" << value << "\"";
        return *this;
    }

    xml_tag& attribute(std::string_view name, std::string_view value)
    {
        *m_stream << ' ' << name << "=\"" << value << "\"";
        return *this;
    }

    xml_tag& text(std::string_view text)
    {
        if (!m_is_parent)
        {
            m_is_parent = true;
            *m_stream << ">";
        }

        *m_stream << text;
        return *this;
    }

    void no_newline()
    {
        m_is_inline = true;
    }

private:
    std::ostream* m_stream{};
    std::string m_name;
    bool m_is_parent{};
    bool m_is_inline{};
};

struct svg_point final
{
    size_t x{};
    size_t y{};
};

struct svg_size final
{
    size_t width{};
    size_t height{};
};

struct svg_rect final
{
    size_t x{};
    size_t y{};
    size_t width{};
    size_t height{};
};

enum class svg_dominant_baseline
{
    baseline,
    middle,
    hanging
};

std::string_view to_string(svg_dominant_baseline value)
{
    switch (value)
    {
        case svg_dominant_baseline::baseline: return "baseline";
        case svg_dominant_baseline::middle:   return "middle";
        case svg_dominant_baseline::hanging:  return "hanging";
        default:                              return "middle";
    }
};

enum class svg_text_anchor
{
    start,
    middle,
    end
};

std::string_view to_string(svg_text_anchor value)
{
    switch (value)
    {
        case svg_text_anchor::start:  return "start";
        case svg_text_anchor::middle: return "middle";
        case svg_text_anchor::end:    return "end";
        default:                      return "start";
    }
};

struct svg_rect_attributes final
{
    std::string fill{"black"};
    std::string stroke{"none"};
    std::string stroke_width{"1"};
};

struct svg_text_attributes final
{
    std::string font_size{"medium"};
    std::string font_family{"sans-serif"};
    std::string font_weight{"normal"};
    std::string fill{"black"};
    std::string stroke{"none"};
    svg_text_anchor text_anchor{svg_text_anchor::start};
    svg_dominant_baseline dominant_baseline{svg_dominant_baseline::middle};
};

class svg_writer final
{
public:
    svg_writer(std::ostream& stream, svg_size size)
        : m_stream{stream}
        , m_size{size}
        , m_svg{m_stream, "svg"}
    {
        m_svg.attribute("width", m_size.width);
        m_svg.attribute("height", m_size.height);
        m_svg.attribute("version", "1.1");
        m_svg.attribute("baseProfile", "full");
        m_svg.attribute("xmlns", "http://www.w3.org/2000/svg");
    }

    void write_background(std::string_view color = "white")
    {
        xml_tag{m_svg, "rect"}
            .attribute("width", "100%")
            .attribute("height", "100%")
            .attribute("fill", color);
    }

    void write_grid(size_t distance, std::string_view color = "whitesmoke")
    {
        for (size_t i = distance; i <= m_size.width; i += distance)
            xml_tag{m_svg, "line"}
                .attribute("x1", i)
                .attribute("x2", i)
                .attribute("y1", 0)
                .attribute("y2", m_size.height)
                .attribute("stroke", color)
                .attribute("stroke-width", 1);

        for (size_t i = distance; i <= m_size.height; i += distance)
            xml_tag{m_svg, "line"}
                .attribute("x1", 0)
                .attribute("x2", m_size.width)
                .attribute("y1", i)
                .attribute("y2", i)
                .attribute("stroke", color)
                .attribute("stroke-width", 1);
    }

    void write_border(std::string_view color = "black")
    {
        xml_tag{m_svg, "rect"}
            .attribute("x", 0)
            .attribute("y", 0)
            .attribute("width", m_size.width)
            .attribute("height", m_size.height)
            .attribute("stroke", color)
            .attribute("fill", "transparent")
            .attribute("stroke-width", 1);
    }

    void write_rect(svg_rect rect, const svg_rect_attributes& attributes)
    {
        xml_tag{m_svg, "rect"}
            .attribute("x", rect.x)
            .attribute("y", rect.y)
            .attribute("width", rect.width)
            .attribute("height", rect.height)
            .attribute("stroke", attributes.stroke)
            .attribute("fill", attributes.fill)
            .attribute("stroke-width", attributes.stroke_width);
    }

    void write_text(svg_point point, const svg_text_attributes& attributes, std::string_view text)
    {
        xml_tag{m_svg, "text"}
            .attribute("x", point.x)
            .attribute("y", point.y)
            .attribute("font-size", attributes.font_size)
            .attribute("font-family", attributes.font_family)
            .attribute("font-weight", attributes.font_weight)
            .attribute("text-anchor", to_string(attributes.text_anchor))
            .attribute("dominant-baseline", to_string(attributes.dominant_baseline))
            .attribute("fill", attributes.fill)
            .attribute("stroke", attributes.stroke)
            .text(text);
    }

private:
    std::ostream& m_stream;
    svg_size m_size;
    xml_tag m_svg;
};

int main()
{
    svg_rect_attributes default_rect;
    default_rect.fill = "whitesmoke";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";

    svg_text_attributes default_text;
    default_text.font_size = "25";
    default_text.font_weight = "bold";

    svg_writer svg{std::cout, {1024, 768}};
    svg.write_background();
    svg.write_grid(50);
    svg.write_rect({50, 50, 300, 50}, default_rect);
    svg.write_text({75, 75}, default_text, "SomeName");
    svg.write_border();
}