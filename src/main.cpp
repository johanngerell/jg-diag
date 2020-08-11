#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <unordered_map>

class box final
{
public:
    box(jg::rect bounds, std::string_view text, size_t id)
        : m_bounds{bounds}
        , m_text{text}
        , m_id{id}
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
    diagram(jg::size extent, size_t capacity = 0)
        : m_extent{extent}
    {
        m_shapes.reserve(capacity);
    }

    const box& add_box(jg::rect bounds, std::string_view text)
    {
        m_shapes.push_back(box{bounds, text, new_id()});
        const auto& back = std::get<box>(m_shapes.back());
        m_shape_ids[back.id()] = m_shapes.size() - 1;

        return back;
    }

    template <typename T1, typename T2>
    void add_line(const T1& source, const T2& target, line_kind kind)
    {
        m_lines.push_back({source.id(), target.id(), kind});
    }

private:
    static size_t new_id()
    {
        static size_t id = 0;
        return ++id;
    }

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

int main()
{
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

    diagram diag{{1024, 768}, 4};

    const box& box1 = diag.add_box({100, 100, 300, 50}, "Box 1");
    const box& box2 = diag.add_box({500, 50, 300, 50}, "Box 2");
    const box& box3 = diag.add_box({550, 300, 300, 50}, "Box 3");
    const box& box4 = diag.add_box({50, 250, 300, 50}, "Box 4");

    diag.add_line(box1, box2, line_kind::filled_arrow);
    diag.add_line(box2, box3, line_kind::filled_arrow);
    diag.add_line(box3, box4, line_kind::filled_arrow);
    diag.add_line(box4, box1, line_kind::filled_arrow);
    
    std::cout << diag;
}