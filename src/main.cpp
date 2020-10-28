#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

class rectangle final
{
public:
    rectangle(jg::rect bounds, std::string_view text)
        : m_bounds{bounds}
        , m_text{text}
    {
        m_anchors[0] = {m_bounds.x                     , m_bounds.y + m_bounds.height / 2};
        m_anchors[1] = {m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2};
        m_anchors[2] = {m_bounds.x + m_bounds.width / 2, m_bounds.y};
        m_anchors[3] = {m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height};
    }

    jg::rect bounds() const
    {
        return m_bounds;
    }

    std::string_view text() const
    {
        return m_text;
    }

    const std::array<jg::point, 4>& anchors() const
    {
        return m_anchors;
    }

private:
    jg::rect m_bounds;
    std::string m_text;
    std::array<jg::point, 4> m_anchors;
};

class rhombus final
{
public:
    rhombus(jg::rect bounds, std::string_view text)
        : m_bounds{bounds}
        , m_text{text}
    {
        m_anchors[0] = {m_bounds.x                     , m_bounds.y + m_bounds.height / 2};
        m_anchors[1] = {m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2};
        m_anchors[2] = {m_bounds.x + m_bounds.width / 2, m_bounds.y};
        m_anchors[3] = {m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height};
    }

    jg::rect bounds() const
    {
        return m_bounds;
    }

    std::string_view text() const
    {
        return m_text;
    }

    const std::array<jg::point, 4>& anchors() const
    {
        return m_anchors;
    }

private:
    jg::rect m_bounds;
    std::string m_text;
    std::array<jg::point, 4> m_anchors;
};

class ellipse final
{
public:
    ellipse(jg::rect bounds, std::string_view text)
        : m_bounds{bounds}
        , m_text{text}
    {
        m_anchors[0] = {m_bounds.x                     , m_bounds.y + m_bounds.height / 2};
        m_anchors[1] = {m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2};
        m_anchors[2] = {m_bounds.x + m_bounds.width / 2, m_bounds.y};
        m_anchors[3] = {m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height};
    }

    jg::rect bounds() const
    {
        return m_bounds;
    }

    std::string_view text() const
    {
        return m_text;
    }

    const std::array<jg::point, 4>& anchors() const
    {
        return m_anchors;
    }

private:
    jg::rect m_bounds;
    std::string m_text;
    std::array<jg::point, 4> m_anchors;
};

using entity_id = size_t;

enum class line_kind
{
    filled_arrow
};

class line final
{
public:
    entity_id source_id{};
    entity_id target_id{};
    line_kind kind{};
};

class diagram final
{
public:
    template <typename T>
    entity_id entity(T&& b)
    {
        const auto id = new_id();
        const auto& shape = m_shapes.insert({id, std::move(b)}).first->second;
        
        std::visit([&](const auto& b)
        {
            const auto& bounds = b.bounds();

            if (bounds.x + bounds.width > m_extent.width - 50)
                m_extent.width = bounds.x + bounds.width + 50;

            if (bounds.y + bounds.height > m_extent.height - 50)
                m_extent.height = bounds.y + bounds.height + 50;
        }, shape);
        
        return id;
    }

    entity_id entity(line&& l)
    {
        m_lines.push_back(std::move(l));
        return 0;
    }

private:
    static entity_id new_id()
    {
        static entity_id id;
        return ++id;
    }

    friend std::ostream& operator<<(std::ostream&, const diagram&);

    jg::size m_extent;
    std::map<entity_id, std::variant<rectangle, rhombus, ellipse>> m_shapes;
    std::vector<line> m_lines;
};

// https://www.bfilipek.com/2018/06/variant.html#overload
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

std::ostream& operator<<(std::ostream& stream, const diagram& diag)
{
    jg::svg_writer svg{stream, diag.m_extent};
    svg.write_background();
    svg.write_grid(50);

    jg::svg_rect_attributes default_rect;
    default_rect.fill = "whitesmoke";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";

    constexpr float font_size = 25;
    jg::svg_text_attributes default_text;
    default_text.font_size = std::to_string(font_size);
    default_text.font_weight = "bold";

    jg::svg_circle_attributes default_circle;
    default_circle.fill = "red";

    for (const auto& [_, shape] : diag.m_shapes)
    {
        const jg::rect bounds = std::visit([&](const auto& b) { return b.bounds(); }, shape);

        std::visit(overload
        {
            [&](const rectangle&)
            {
                svg.write_rect({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            },
            [&](const rhombus&)
            {
                svg.write_rhombus({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            },
            [&](const ellipse&)
            {
                svg.write_ellipse({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, bounds.width / 2, bounds.height / 2, default_rect);
            }
        }, shape);

        std::visit([&](const auto& b)
        {
            svg.write_text({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, default_text, b.text());
            for (const auto& anchor : b.anchors())
                svg.write_circle({anchor.x, anchor.y}, 5, default_circle);
        }, shape);
    };

    jg::svg_line_attributes default_line;
    default_line.stroke_width = "3";

    for (const auto& line : diag.m_lines)
    {
        std::array<jg::point, 4> source_anchors;
        std::visit([&] (const auto& source)
        {
            source_anchors = source.anchors();
        }, diag.m_shapes.find(line.source_id)->second);

        std::array<jg::point, 4> target_anchors;
        std::visit([&] (const auto& target)
        {
            target_anchors = target.anchors();
        }, diag.m_shapes.find(line.target_id)->second);

        std::pair<jg::point, jg::point> anchors;
        float shortest_distance = std::numeric_limits<float>::max();

        for (const auto& source_anchor : source_anchors)
        {
            for (const auto& target_anchor : target_anchors)
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

    const auto& entity1 = diag.entity(rectangle{{100, 100, 300,  50}, "Rectangle"});
    const auto& entity2 = diag.entity(ellipse  {{500,  50, 300, 100}, "Ellipse 1"});
    const auto& entity3 = diag.entity(rhombus{{550, 400, 300, 100}, "Rhombus"});
    const auto& entity4 = diag.entity(ellipse  {{ 50, 250, 300,  50}, "Ellipse 2"});

    diag.entity(line{entity1, entity2, line_kind::filled_arrow});
    diag.entity(line{entity2, entity3, line_kind::filled_arrow});
    diag.entity(line{entity3, entity4, line_kind::filled_arrow});
    diag.entity(line{entity4, entity1, line_kind::filled_arrow});
    
    std::cout << diag;
}
