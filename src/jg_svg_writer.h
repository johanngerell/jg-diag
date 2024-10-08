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

struct svg_paint_attributes final
{
    std::string fill;
    std::string stroke;
    std::string stroke_width;
};

struct svg_font_attributes final
{
    std::string size;   // medium | <number>
    std::string family; // serif | sans-serif | cursive | fantasy | monospace
    std::string weight; // normal | bold | bolder | lighter | <number>
    std::string style;  // normal italic oblique
};

struct svg_text_attributes final
{
    svg_font_attributes font;
    svg_paint_attributes paint;
    svg_text_anchor text_anchor;
    svg_dominant_baseline dominant_baseline;
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
        for (float f = distance; f < m_size.width; f += distance)
            write_line({f, 0}, {f, m_size.height}, {"none", std::string(color), "1"});

        for (float f = distance; f < m_size.height; f += distance)
            write_line({0, f}, {m_size.width, f}, {"none", std::string(color), "1"});
    }

    void write_title(std::string_view title)
    {
        if (title.empty())
            return;

        constexpr float font_size = 25;
        svg_text_attributes attributes;
        attributes.text_anchor = svg_text_anchor::start;
        attributes.font = {std::to_string(font_size), "sans-serif", "bold", "normal"};
        attributes.text_anchor = svg_text_anchor::start;
        attributes.dominant_baseline = svg_dominant_baseline::middle;

        write_text({font_size / 2, font_size}, title, attributes);
    }

    void write_border()
    {
        auto tag = xml_writer::child_element(m_root, "rect");
        tag.write_attribute("x", 0);
        tag.write_attribute("y", 0);
        tag.write_attribute("width", m_size.width);
        tag.write_attribute("height", m_size.height);
        tag.write_attribute("fill", "transparent");
        tag.write_attribute("stroke", "black");
        tag.write_attribute("stroke-width", 1);
    }

    void write_line(jg::point point1, jg::point point2, const svg_paint_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "line");
        tag.write_attribute("x1", point1.x);
        tag.write_attribute("y1", point1.y);
        tag.write_attribute("x2", point2.x);
        tag.write_attribute("y2", point2.y);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_arrow(jg::point point1, jg::point point2, const svg_paint_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "line");
        tag.write_attribute("x1", point1.x);
        tag.write_attribute("y1", point1.y);

        const float dx = point2.x - point1.x;
        const float dy = point2.y - point1.y;
        const float distance = std::hypotf(dx, dy);
        const float ddx = m_arrowhead_length * dx / distance;
        const float ddy = m_arrowhead_length * dy / distance;

        tag.write_attribute("x2", point2.x - ddx);
        tag.write_attribute("y2", point2.y - ddy);

        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("marker-end", "url(#arrowhead)");
    }

    void write_rect(jg::rect rect, const svg_paint_attributes& attributes)
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

    void write_rhombus(jg::rect rect, const svg_paint_attributes& attributes)
    {
        const jg::point point1{rect.x                 , rect.y + rect.height / 2};
        const jg::point point2{rect.x + rect.width / 2, rect.y};
        const jg::point point3{rect.x + rect.width    , rect.y + rect.height / 2};
        const jg::point point4{rect.x + rect.width / 2, rect.y + rect.height};

        std::ostringstream path;
        path << "M"  << point1.x << " " << point1.y;
        path << " L" << point2.x << " " << point2.y;
        path << " L" << point3.x << " " << point3.y;
        path << " L" << point4.x << " " << point4.y;
        path << " Z";
        
        auto tag = xml_writer::child_element(m_root, "path");
        tag.write_attribute("d", path.str());
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("stroke-linejoin", "bevel");
    }

    void write_parallelogram(jg::rect rect, const svg_paint_attributes& attributes)
    {
        const jg::point point1{rect.x + rect.height             , rect.y};
        const jg::point point2{rect.x + rect.width              , rect.y};
        const jg::point point3{rect.x + rect.width - rect.height, rect.y + rect.height};
        const jg::point point4{rect.x                           , rect.y + rect.height};

        std::ostringstream path;
        path << "M"  << point1.x << " " << point1.y;
        path << " L" << point2.x << " " << point2.y;
        path << " L" << point3.x << " " << point3.y;
        path << " L" << point4.x << " " << point4.y;
        path << " Z";
        
        auto tag = xml_writer::child_element(m_root, "path");
        tag.write_attribute("d", path.str());
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke-width", attributes.stroke_width);
        tag.write_attribute("stroke-linejoin", "bevel");
    }

    void write_text(jg::point point, std::string_view text, const svg_text_attributes& attributes)
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
        tag.write_attribute("fill", attributes.paint.fill);
        tag.write_attribute("stroke", attributes.paint.stroke);
        tag.write_text(text);
    }

    void write_circle(jg::point point, float radius, const svg_paint_attributes& attributes)
    {
        auto tag = xml_writer::child_element(m_root, "circle");
        tag.write_attribute("cx", point.x);
        tag.write_attribute("cy", point.y);
        tag.write_attribute("r", radius);
        tag.write_attribute("fill", attributes.fill);
        tag.write_attribute("stroke", attributes.stroke);
        tag.write_attribute("stroke-width", attributes.stroke_width);
    }

    void write_ellipse(jg::point point, float xradius, float yradius, const svg_paint_attributes& attributes)
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

    void write_comment(std::string_view comment)
    {
        auto tag = xml_writer::child_element(m_root, "");
        tag.write_comment(comment);
    }

private:
    std::ostream& m_stream;
    jg::size m_size;
    xml_writer m_root;
    float m_arrowhead_length{20.0f};
};

} // namespace jg
