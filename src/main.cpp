#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <unordered_map>

struct point final
{
    size_t x{};
    size_t y{};
};

struct rect final
{
    size_t x{};
    size_t y{};
    size_t width{};
    size_t height{};
};

class box final
{
public:
    box(rect r, std::string text, size_t id)
        : m_rect{std::move(r)}
        , m_text{std::move(text)}
        , m_id{id}
    {
        m_anchor_points.push_back({r.x,           r.y + r.height / 2});
        m_anchor_points.push_back({r.x + r.width, r.y + r.height / 2});

        m_anchor_points.push_back({r.x + r.width / 2, r.y});
        m_anchor_points.push_back({r.x + r.width / 2, r.y + r.height});
    }

//private:
    size_t m_id;
    rect m_rect;
    std::string m_text;
    size_t m_text_size{25};
    size_t m_text_offset{25};
    std::vector<point> m_anchor_points;
};

class relationship final
{
public:
    size_t source{};
    size_t target{};
    size_t kind{};
};

class diag_writer final
{
    jg::svg_writer m_svg{std::cout, {1024, 768}};
    std::vector<std::variant<box>> m_shapes;
    std::unordered_map<size_t, size_t> m_shape_ids; // id, index
    std::vector<relationship> m_relationships;

    friend std::ostream& operator<<(std::ostream&, diag_writer&);

public:
    template <typename T>
    void add_shape(T&& shape)
    {
        m_shapes.push_back(std::move(shape));
        m_shape_ids[std::get<box>(m_shapes.back()).m_id] = m_shapes.size() - 1;
    }

    void add_relation(size_t source_id, size_t target_id)
    {
        m_relationships.push_back({source_id, target_id});
    }
};

std::ostream& operator<<(std::ostream& stream, diag_writer& diag)
{
    jg::svg_writer svg{stream, {1024, 768}};
    svg.write_background();
    svg.write_grid(50);

    jg::svg_rect_attributes default_rect;
    default_rect.fill = "whitesmoke";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";

    jg::svg_text_attributes default_text;
    default_text.font_size = "25";
    default_text.font_weight = "bold";

    jg::svg_circle_attributes default_circle;
    default_circle.fill = "red";

    for (const auto& shape : diag.m_shapes)
    {
        std::visit([&](const box& b) {
            svg.write_rect({b.m_rect.x, b.m_rect.y, b.m_rect.width, b.m_rect.height}, default_rect);
            svg.write_text({b.m_rect.x + 25, b.m_rect.y + b.m_rect.height / 2}, default_text, b.m_text);

            for (const auto& anchor : b.m_anchor_points)
                svg.write_circle({anchor.x, anchor.y}, 5, default_circle);
        },
        shape);
    }

    jg::svg_line_attributes default_line;
    default_line.stroke_width = "3";

    for (const auto& relation : diag.m_relationships)
    {
        const auto& source = std::get<box>(diag.m_shapes[diag.m_shape_ids[relation.source]]);
        const auto& target = std::get<box>(diag.m_shapes[diag.m_shape_ids[relation.target]]);

        std::pair<point, point> anchors;
        float shortest_distance = std::numeric_limits<float>::max();

        for (const auto& source_anchor : source.m_anchor_points)
        {
            for (const auto& target_anchor : target.m_anchor_points)
            {
                const float dx = (float)target_anchor.x - (float)source_anchor.x;
                const float dy = (float)target_anchor.y - (float)source_anchor.y;
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

    diag_writer diag;
    
    diag.add_shape(box{{50, 50, 300, 50}, "Box 1", 1});
    diag.add_shape(box{{500, 50, 300, 50}, "Box 2", 2});
    diag.add_shape(box{{550, 300, 300, 50}, "Box 3", 3});
    diag.add_shape(box{{50, 250, 300, 50}, "Box 4", 4});

    diag.add_relation(1, 2);
    diag.add_relation(2, 3);
    diag.add_relation(3, 4);
    diag.add_relation(4, 1);
    
    std::cout << diag;
}