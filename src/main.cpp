#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <unordered_map>

class box final
{
public:
    box(jg::rect bounds, std::string_view text, size_t id)
        : m_id{id}
        , m_bounds{bounds}
        , m_text{text}
    {
        m_anchors.push_back({m_bounds.x                     , m_bounds.y + m_bounds.height / 2});
        m_anchors.push_back({m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2});
        m_anchors.push_back({m_bounds.x + m_bounds.width / 2, m_bounds.y});
        m_anchors.push_back({m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height});
    }

    box(box&&) = default;
    box& operator=(box&&) = default;

    jg::rect bounds() const
    {
        return m_bounds;
    }

    std::string_view text() const
    {
        return m_text;
    }

    size_t id() const
    {
        return m_id;
    }

    const std::vector<jg::point>& anchors() const
    {
        return m_anchors;
    }

private:
    size_t m_id;
    jg::rect m_bounds;
    std::string m_text;
    std::vector<jg::point> m_anchors;
};

enum class line_kind
{
    filled_arrow
};

class line_info final
{
public:
    size_t source{};
    size_t target{};
    line_kind kind{};
};

class diagram final
{
public:
    void add_box(box&& b)
    {
        m_shapes.emplace_back(std::move(b));
        const auto& back = std::get<box>(m_shapes.back());
        m_shape_ids[back.id()] = m_shapes.size() - 1;

        const auto bounds = back.bounds();

        if (bounds.x + bounds.width > m_extent.width - 50)
            m_extent.width = bounds.x + bounds.width + 50;

        if (bounds.y + bounds.height > m_extent.height - 50)
            m_extent.height = bounds.y + bounds.height + 50;
    }

    void add_line(size_t source_id, size_t target_id, line_kind kind)
    {
        m_lines.push_back({source_id, target_id, kind});
    }

private:
    friend std::ostream& operator<<(std::ostream&, const diagram&);

    jg::size m_extent;
    std::vector<std::variant<box>> m_shapes;
    std::unordered_map<size_t, size_t> m_shape_ids; // id, index
    std::vector<line_info> m_lines;
};

std::ostream& operator<<(std::ostream& stream, const diagram& diag)
{
    jg::svg_writer svg{stream, diag.m_extent};
    svg.write_background();
    svg.write_grid(50);

    jg::svg_rect_attributes default_rect;
    default_rect.fill = "whitesmoke";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";

    constexpr float text_offset = 12.5; // 25 / 2
    jg::svg_text_attributes default_text;
    default_text.font_size = "25";
    default_text.font_weight = "bold";

    jg::svg_circle_attributes default_circle;
    default_circle.fill = "red";

    for (const auto& shape : diag.m_shapes)
    {
        std::visit([&](const auto& b) {
            const auto bounds = b.bounds();
            svg.write_rect({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            svg.write_text({bounds.x + text_offset, bounds.y + bounds.height / 2}, default_text, b.text());

            for (const auto& anchor : b.anchors())
                svg.write_circle({anchor.x, anchor.y}, 5, default_circle);
        },
        shape);
    }

    jg::svg_line_attributes default_line;
    default_line.stroke_width = "3";

    for (const auto& line : diag.m_lines)
    {
        const auto& source = std::get<box>(diag.m_shapes[diag.m_shape_ids.at(line.source)]);
        const auto& target = std::get<box>(diag.m_shapes[diag.m_shape_ids.at(line.target)]);

        std::pair<jg::point, jg::point> anchors;
        float shortest_distance = std::numeric_limits<float>::max();

        for (const auto& source_anchor : source.anchors())
        {
            for (const auto& target_anchor : target.anchors())
            {
                const float dx = target_anchor.x - source_anchor.x;
                const float dy = target_anchor.y - source_anchor.y;
                const float distance = std::hypotf(dx, dy);

                if (distance < shortest_distance)
                {
                    shortest_distance = distance;
                    anchors = {source_anchor, target_anchor};
                }
            }
        }

        svg.write_arrow({anchors.first.x, anchors.first.y},
                        {anchors.second.x, anchors.second.y},
                        default_line);
    }

    svg.write_border();

    return stream;
}

/*
    const char* json = R"json(
    {
        "shapes": [
            {
                "type": "box",
                "rect": [50, 50, 300, 50],
                "text": "Box 1"
            },
            {
                "type": "box",
                "rect": [500, 50, 300, 50],
                "text": "Box 2"
            },
            {
                "type": "box",
                "rect": [550, 300, 300, 50],
                "text": "Box 3"
            },
            {
                "type": "box",
                "rect": [50, 250, 300, 50],
                "text": "Box 4"
            }
        ],
        "lines": [
            {
                "source": 0,
                "target": 1,
                "kind": "black_arrow"
            },
            {
                "source": 1,
                "target": 2,
                "kind": "black_arrow"
            },
            {
                "source": 2,
                "target": 3,
                "kind": "black_arrow"
            },
            {
                "source": 3,
                "target": 4,
                "kind": "black_arrow"
            }
        ]
    }
    )json";
    
    std::cout << json;
*/

int main()
{
    diagram diag;

    diag.add_box({{100, 100, 300, 50}, "Box 1", 1});
    diag.add_box({{500,  50, 300, 50}, "Box 2", 2});
    diag.add_box({{550, 300, 300, 50}, "Box 3", 3});
    diag.add_box({{ 50, 250, 300, 50}, "Box 4", 4});

    diag.add_line(1, 2, line_kind::filled_arrow);
    diag.add_line(2, 3, line_kind::filled_arrow);
    diag.add_line(3, 4, line_kind::filled_arrow);
    diag.add_line(4, 1, line_kind::filled_arrow);
    
    std::cout << diag;
}