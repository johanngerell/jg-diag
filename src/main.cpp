#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <unordered_map>

class box final
{
public:
    box(jg::rect bounds, std::string text, size_t id)
        : m_bounds{std::move(bounds)}
        , m_text{std::move(text)}
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

enum class relationship_kind
{
    filled_arrow
};

class relationship final
{
public:
    size_t source{};
    size_t target{};
    relationship_kind kind{};
};

class diagram final
{
    jg::size m_size;
    std::vector<std::variant<box>> m_shapes;
    std::unordered_map<size_t, size_t> m_shape_ids; // id, index
    std::vector<relationship> m_relationships;

    friend std::ostream& operator<<(std::ostream&, const diagram&);

public:
    diagram(jg::size size)
        : m_size{std::move(size)}
    {}

    template <typename T>
    void add_shape(T&& shape)
    {
        m_shapes.push_back(std::move(shape));
        m_shape_ids[std::get<box>(m_shapes.back()).id()] = m_shapes.size() - 1;
    }

    void add_relation(size_t source_id, size_t target_id, relationship_kind kind)
    {
        m_relationships.push_back({source_id, target_id, kind});
    }
};

std::ostream& operator<<(std::ostream& stream, const diagram& diag)
{
    jg::svg_writer svg{stream, diag.m_size};
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

    for (const auto& relation : diag.m_relationships)
    {
        const auto& source = std::get<box>(diag.m_shapes[diag.m_shape_ids.at(relation.source)]);
        const auto& target = std::get<box>(diag.m_shapes[diag.m_shape_ids.at(relation.target)]);

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
                "text": "Box 1",
                "id": 1
            },
            {
                "type": "box",
                "rect": [500, 50, 300, 50],
                "text": "Box 2",
                "id": 2
            },
            {
                "type": "box",
                "rect": [550, 300, 300, 50],
                "text": "Box 3",
                "id": 3
            },
            {
                "type": "box",
                "rect": [50, 250, 300, 50],
                "text": "Box 4",
                "id": 4
            }
        ],
        "relations": [
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
            },
            {
                "source": 4,
                "target": 1,
                "kind": "black_arrow"
            }
        ]
    }
    )json";
    
    std::cout << json;
    */

    diagram diag{{1024, 768}};

    diag.add_shape(box{{50, 50, 300, 50}, "Box 1", 1});
    diag.add_shape(box{{500, 50, 300, 50}, "Box 2", 2});
    diag.add_shape(box{{550, 300, 300, 50}, "Box 3", 3});
    diag.add_shape(box{{50, 250, 300, 50}, "Box 4", 4});

    diag.add_relation(1, 2, relationship_kind::filled_arrow);
    diag.add_relation(2, 3, relationship_kind::filled_arrow);
    diag.add_relation(3, 4, relationship_kind::filled_arrow);
    diag.add_relation(4, 1, relationship_kind::filled_arrow);
    
    std::cout << diag;
}