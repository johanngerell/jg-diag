#include <jg_verify.h>
#include "jg_xml_writer.h"

namespace jg
{

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
        default: verify(false);               return "unknown";
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
        default: verify(false);       return "unknown";
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
        , m_root{xml_writer::root_element(m_stream, "svg")}
    {
        m_root.write_attribute("width", m_size.width);
        m_root.write_attribute("height", m_size.height);
        m_root.write_attribute("version", "1.1");
        m_root.write_attribute("baseProfile", "full");
        m_root.write_attribute("xmlns", "http://www.w3.org/2000/svg");
    }

    void write_background(std::string_view color = "white")
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("width", "100%");
        tag.write_attribute("height", "100%");
        tag.write_attribute("fill", color);
    }

    void write_grid(size_t distance, std::string_view color = "whitesmoke")
    {
        for (size_t i = distance; i <= m_size.width; i += distance)
        {
            auto tag = xml_writer::child_element(m_root, "line");
            tag.write_attribute("x1", i);
            tag.write_attribute("x2", i);
            tag.write_attribute("y1", 0);
            tag.write_attribute("y2", m_size.height);
            tag.write_attribute("stroke", color);
            tag.write_attribute("stroke-width", 1);
        }

        for (size_t i = distance; i <= m_size.height; i += distance)
        {
            auto tag = xml_writer::child_element(m_root, "line");
            tag.write_attribute("x1", 0);
            tag.write_attribute("x2", m_size.width);
            tag.write_attribute("y1", i);
            tag.write_attribute("y2", i);
            tag.write_attribute("stroke", color);
            tag.write_attribute("stroke-width", 1);
        }
    }

    void write_border(std::string_view color = "black")
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("x", 0);
        tag.write_attribute("y", 0);
        tag.write_attribute("width", m_size.width);
        tag.write_attribute("height", m_size.height);
        tag.write_attribute("stroke", color);
        tag.write_attribute("fill", "transparent");
        tag.write_attribute("stroke-width", 1);
    }

    void write_rect(svg_rect rect, const svg_rect_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("x", rect.x);
        tag.write_attribute("y", rect.y);
        tag.write_attribute("width", rect.width);
        tag.write_attribute("height", rect.height);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_text(svg_point point, const svg_text_attributes& attributes, std::string_view text)
    {
        auto tag = xml_writer::child_element(m_root, "text");
        tag.write_attribute("x", point.x);
        tag.write_attribute("y", point.y);
        tag.write_attribute("font-size", attributes.font_size);
        tag.write_attribute("font-family", attributes.font_family);
        tag.write_attribute("font-weight", attributes.font_weight);
        tag.write_attribute("text-anchor", to_string(attributes.text_anchor));
        tag.write_attribute("dominant-baseline", to_string(attributes.dominant_baseline));
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_text(text);
    }

private:
    std::ostream& m_stream;
    svg_size m_size;
    xml_writer m_root;
};

} // namespace jg
