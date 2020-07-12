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

struct entity final
{
    std::string name;
    entity* dependency{};
};

static size_t new_id()
{
    static size_t last = 0;
    return ++last;
}

class box final
{
public:
    box(rect r, std::string text)
        : m_rect{std::move(r)}
        , m_text{std::move(text)}
    {
        m_anchor_points.push_back({r.x,           r.y + r.height / 2});
        m_anchor_points.push_back({r.x + r.width, r.y + r.height / 2});

        m_anchor_points.push_back({r.x + r.width / 2, r.y});
        m_anchor_points.push_back({r.x + r.width / 2, r.y + r.height});
    }

//private:
    size_t id{new_id()};
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

public:
    template <typename T>
    size_t add_shape(T&& shape)
    {
        m_shapes.push_back(std::move(shape));
        const auto id = std::get<box>(m_shapes.back()).id;
        m_shape_ids[id] = m_shapes.size() - 1;

        return id;
    }

    void add_relation(size_t source_id, size_t target_id)
    {
        m_relationships.push_back({source_id, target_id});
    }

    void write()
    {
        jg::svg_writer svg{std::cout, {1024, 768}};
        svg.write_background();
        svg.write_grid(50);

        jg::svg_rect_attributes default_rect;
        default_rect.fill = "whitesmoke";
        default_rect.stroke = "black";
        default_rect.stroke_width = "3";

        jg::svg_text_attributes default_text;
        default_text.font_size = "25";
        default_text.font_weight = "bold";
        
        for (auto& shape : m_shapes)
        {
            std::visit([&] (box& b)
            {
                svg.write_rect({b.m_rect.x, b.m_rect.y, b.m_rect.width, b.m_rect.height}, default_rect);
                svg.write_text({b.m_rect.x + 25, b.m_rect.y + b.m_rect.height / 2 }, default_text, b.m_text);
            }, shape);
        }

        jg::svg_line_attributes default_line;
        default_line.stroke_width = "3";

        for (auto& relation : m_relationships)
        {
            auto& source = std::get<box>(m_shapes[m_shape_ids[relation.source]]);
            auto& target = std::get<box>(m_shapes[m_shape_ids[relation.target]]);

            std::pair<point, point> anchors;
            float shortest_distance = std::numeric_limits<float>::max();

            for (auto& source_anchor : source.m_anchor_points)
            {
                for (auto& target_anchor : target.m_anchor_points)
                {
                    float dx = (float)target_anchor.x - (float)source_anchor.x;
                    float dy = (float)target_anchor.y - (float)source_anchor.y;
                    float dx2 = dx * dx;
                    float dy2 = dy * dy;
                    float distance = sqrt(dx2 + dy2);

                    if (distance < shortest_distance)
                    {
                        shortest_distance = distance;
                        anchors.first = source_anchor;
                        anchors.second = target_anchor;
                    }
                }
            }

            svg.write_arrow({anchors.first.x, anchors.first.y},
                            {anchors.second.x, anchors.second.y},
                            default_line);
        }

        svg.write_border();
    }
};

int main()
{
    diag_writer diag;
    
    const auto id1 = diag.add_shape(box{{50, 50, 300, 50}, "Box 1"});
    const auto id2 = diag.add_shape(box{{500, 50, 300, 50}, "Box 2"});
    const auto id3 = diag.add_shape(box{{500, 250, 300, 50}, "Box 3"});
    const auto id4 = diag.add_shape(box{{50, 250, 300, 50}, "Box 4"});

    diag.add_relation(id1, id2);
    diag.add_relation(id2, id3);
    diag.add_relation(id3, id4);
    diag.add_relation(id4, id1);
    
    diag.write();
}