#pragma once

#include <sstream>
#include <type_traits>
#include <cmath>
#include <jg_verify.h>
#include "jg_xml_writer.h"
#include "jg_coordinates.h"

namespace jg
{

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

std::ostream& operator<<(std::ostream& stream, svg_dominant_baseline dominant_baseline)
{
    return (stream << to_string(dominant_baseline));
}

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

std::ostream& operator<<(std::ostream& stream, svg_text_anchor text_anchor)
{
    return (stream << to_string(text_anchor));
}

template <typename T>
std::string attribute_value(T value)
{
    if constexpr (std::is_constructible_v<std::string, T>)
        return std::string{value};
    else
        return std::to_string(value);
}

struct svg_shape_attributes final
{
    std::string fill{"black"};
    std::string stroke{"black"};
    std::string stroke_width{"1"};

    svg_shape_attributes() = default;

    template <typename TFill, typename TStroke, typename TStrokeWidth>
    svg_shape_attributes(TFill fill, TStroke stroke, TStrokeWidth stroke_width)
        : fill{attribute_value(fill)}
        , stroke{attribute_value(stroke)}
        , stroke_width{attribute_value(stroke_width)}
    {}
};

struct svg_font_attributes final
{
    std::string size{"medium"};
    std::string family{"sans-serif"}; // serif | sans-serif | cursive | fantasy | monospace
    std::string weight{"normal"}; // normal | bold | bolder | lighter | <number>
    std::string style{"normal"}; // normal italic oblique

    svg_font_attributes() = default;

    template <typename TSize>
    svg_font_attributes(TSize size, std::string_view family, std::string_view weight, std::string_view style)
        : size{attribute_value(size)}
        , family{attribute_value(family)}
        , weight{attribute_value(weight)}
        , style{attribute_value(style)}
    {}
};

struct svg_text_attributes final
{
    svg_font_attributes font;
    svg_shape_attributes shape;
    svg_text_anchor text_anchor{svg_text_anchor::middle};
    svg_dominant_baseline dominant_baseline{svg_dominant_baseline::middle};
};

class svg_writer final
{
public:
    svg_writer(std::ostream& stream, jg::size size)
        : m_stream{stream}
        , m_size{size}
        , m_root{xml_writer::root_element(m_stream, "svg")}
    {
        m_root.write_attribute("width", m_size.width);
        m_root.write_attribute("height", m_size.height);
        m_root.write_attribute("version", "1.1");
        m_root.write_attribute("baseProfile", "full");
        m_root.write_attribute("xmlns", "http://www.w3.org/2000/svg");

        auto defs = xml_writer::child_element(m_root, "defs");
        
        auto marker = xml_writer::child_element(defs, "marker");
        marker.write_attribute("id", "arrowhead");
        marker.write_attribute("markerWidth", m_arrowhead_length);
        marker.write_attribute("markerHeight", m_arrowhead_length);
        marker.write_attribute("refX", "0");
        marker.write_attribute("refY", m_arrowhead_length / 2);
        marker.write_attribute("orient", "auto");
        marker.write_attribute("markerUnits", "userSpaceOnUse");

        std::ostringstream points;
        points << "0 0, ";
        points << m_arrowhead_length << " " << m_arrowhead_length / 2;
        points << ", 0 " << m_arrowhead_length;

        auto polygon = xml_writer::child_element(marker, "polygon");
        polygon.write_attribute("points", points.str());
    }

    void write_background(std::string_view color = "white")
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("width", "100%");
        tag.write_attribute("height", "100%");
        tag.write_attribute("fill", color);
    }

    void write_grid(float distance, std::string_view color = "whitesmoke")
    {
        for (float f = distance; f <= m_size.width; f += distance)
            write_line({f, 0}, {f, m_size.height}, {"none", color, 1});

        for (float f = distance; f <= m_size.height; f += distance)
            write_line({0, f}, {m_size.width, f}, {"none", color, 1});
    }

    void write_title(std::string_view title)
    {
        if (title.empty())
            return;

        constexpr float font_size = 25;
        svg_text_attributes attributes;
        attributes.shape.stroke = "none";
        attributes.shape.stroke_width = "";
        attributes.text_anchor = svg_text_anchor::start;
        attributes.font = {font_size, "sans-serif", "bold", "normal"};

        write_text({font_size / 2, font_size}, attributes, title);
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

    void write_line(jg::point p1, jg::point p2, const svg_shape_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "line");
        tag.write_attribute("x1", p1.x);
        tag.write_attribute("y1", p1.y);
        tag.write_attribute("x2", p2.x);
        tag.write_attribute("y2", p2.y);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_arrow(jg::point p1, jg::point p2, const svg_shape_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "line");
        tag.write_attribute("x1", p1.x);
        tag.write_attribute("y1", p1.y);

        const float dx = (float)p2.x - (float)p1.x;
        const float dy = (float)p2.y - (float)p1.y;
        const float distance = std::hypotf(dx, dy);
        const float ddx = (float)m_arrowhead_length * dx / distance;
        const float ddy = (float)m_arrowhead_length * dy / distance;

        tag.write_attribute("x2", (size_t)((float)p2.x - ddx));
        tag.write_attribute("y2", (size_t)((float)p2.y - ddy));

        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("marker-end", "url(#arrowhead)");
    }

    void write_rect(jg::rect rect, const svg_shape_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("x", rect.x);
        tag.write_attribute("y", rect.y);
        tag.write_attribute("width", rect.width);
        tag.write_attribute("height", rect.height);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_rhombus(jg::rect rect, const svg_shape_attributes& attributes)
    {
        const jg::point p1{rect.x, rect.y + rect.height / 2};
        const jg::point p2{rect.x + rect.width / 2, rect.y};
        const jg::point p3{rect.x + rect.width, rect.y + rect.height / 2};
        const jg::point p4{rect.x + rect.width / 2, rect.y + rect.height};

        std::ostringstream path;
        path << "M"  << p1.x << " " << p1.y;
        path << " L" << p2.x << " " << p2.y;
        path << " L" << p3.x << " " << p3.y;
        path << " L" << p4.x << " " << p4.y;
        path << " Z";
        
        auto tag = xml_writer::child_element(m_root, "path");
        tag.write_attribute("d", path.str());
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("stroke-linejoin", "bevel");
    }

    void write_parallelogram(jg::rect rect, const svg_shape_attributes& attributes)
    {
        const jg::point p1{rect.x + rect.height, rect.y};
        const jg::point p2{rect.x + rect.width, rect.y};
        const jg::point p3{rect.x + rect.width - rect.height, rect.y + rect.height};
        const jg::point p4{rect.x, rect.y + rect.height};

        std::ostringstream path;
        path << "M"  << p1.x << " " << p1.y;
        path << " L" << p2.x << " " << p2.y;
        path << " L" << p3.x << " " << p3.y;
        path << " L" << p4.x << " " << p4.y;
        path << " Z";
        
        auto tag = xml_writer::child_element(m_root, "path");
        tag.write_attribute("d", path.str());
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("stroke-linejoin", "bevel");
    }

    void write_text(jg::point point, const svg_text_attributes& attributes, std::string_view text)
    {
        auto tag = xml_writer::child_element(m_root, "text");
        tag.write_attribute("x", point.x);
        tag.write_attribute("y", point.y);
        tag.write_attribute("font-size", attributes.font.size);
        tag.write_attribute("font-family", attributes.font.family);
        tag.write_attribute("font-weight", attributes.font.weight);
        tag.write_attribute("font-style", attributes.font.style);
        tag.write_attribute("text-anchor", to_string(attributes.text_anchor));
        tag.write_attribute("dominant-baseline", to_string(attributes.dominant_baseline));
        tag.write_attribute("fill", attributes.shape.fill);
        tag.write_attribute("stroke", attributes.shape.stroke);
        tag.write_text(text);
    }

    void write_circle(jg::point point, size_t radius, const svg_shape_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "circle");
        tag.write_attribute("cx", point.x);
        tag.write_attribute("cy", point.y);
        tag.write_attribute("r", radius);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_ellipse(jg::point point, size_t xradius, size_t yradius, const svg_shape_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "ellipse");
        tag.write_attribute("cx", point.x);
        tag.write_attribute("cy", point.y);
        tag.write_attribute("rx", xradius);
        tag.write_attribute("ry", yradius);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

private:
    std::ostream& m_stream;
    jg::size m_size;
    xml_writer m_root;
    size_t m_arrowhead_length{20};
};

} // namespace jg
