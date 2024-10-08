#include <array>
#include <map>
#include <unordered_map>
#include <variant>
#include <vector>
#include "jg_svg_writer.h"

namespace jg
{

using anchor_array = std::array<jg::point, 4>;

template <typename TAnchorPolicy>
class shape final
{
public:
    shape(jg::rect bounds, std::string_view text)
        : m_bounds{bounds}
        , m_text{text}
    {}

    jg::rect bounds() const
    {
        return m_bounds;
    }

    std::string_view text() const
    {
        return m_text;
    }

    anchor_array anchors() const
    {
        return TAnchorPolicy::anchors(m_bounds);
    }

private:
    jg::rect m_bounds;
    std::string m_text;
};

//     x
//  x     x
//     x
struct rectangle_anchors final
{
    static anchor_array anchors(const jg::rect& bounds)
    {
        return
        {
            jg::point{bounds.x                   , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width    , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width / 2, bounds.y},
            jg::point{bounds.x + bounds.width / 2, bounds.y + bounds.height}
        };
    }
};

using rectangle = shape<rectangle_anchors>;

//     x
//  x     x
//     x
struct rhombus_anchors final
{
    static anchor_array anchors(const jg::rect& bounds)
    {
        return
        {
            jg::point{bounds.x                   , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width    , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width / 2, bounds.y},
            jg::point{bounds.x + bounds.width / 2, bounds.y + bounds.height}
        };
    }
};

using rhombus = shape<rhombus_anchors>;

//       x
//  x      x
//    x
struct parallelogram_anchors final
{
    static anchor_array anchors(const jg::rect& bounds)
    {
        return
        {
            jg::point{bounds.x + bounds.height / 2,                bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width / 2,                 bounds.y},
            jg::point{bounds.x + bounds.width - bounds.height / 2, bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width / 2,                 bounds.y + bounds.height}
        };
    }
};

using parallelogram = shape<parallelogram_anchors>;

//      x
//  x       x
//      x
struct ellipse_anchors final
{
    static anchor_array anchors(const jg::rect& bounds)
    {
        return
        {
            jg::point{bounds.x                   , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width    , bounds.y + bounds.height / 2},
            jg::point{bounds.x + bounds.width / 2, bounds.y},
            jg::point{bounds.x + bounds.width / 2, bounds.y + bounds.height}
        };
    }
};

using ellipse = shape<ellipse_anchors>;

//     x
//  x     x
//     x
struct circle_anchors final
{
    static anchor_array anchors(const jg::rect& bounds)
    {
        const auto diameter = std::min(bounds.width, bounds.height);

        return
        {
            jg::point{bounds.x               , bounds.y + diameter / 2},
            jg::point{bounds.x + diameter    , bounds.y + diameter / 2},
            jg::point{bounds.x + diameter / 2, bounds.y},
            jg::point{bounds.x + diameter / 2, bounds.y + diameter}
        };
    }
};

using circle = shape<circle_anchors>;

using item_id = size_t;

enum class line_kind
{
    filled_arrow
};

class line final
{
public:
    item_id source_id{};
    item_id target_id{};
    line_kind kind{};
};

// https://www.bfilipek.com/2018/06/variant.html#overload
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

class diagram final
{
public:
    diagram(std::string_view title = "")
        : m_title{title}
    {}

    template <typename T>
    item_id add_item(T&& item)
    {
        const auto id = new_id();
        
        std::visit([&](const auto& shape)
        {
            const auto& bounds = shape.bounds();

            if (bounds.x + bounds.width > m_size.width - 50)
                m_size.width = bounds.x + bounds.width + 50;

            if (bounds.y + bounds.height > m_size.height - 50)
                m_size.height = bounds.y + bounds.height + 50;

        }, m_items.insert({id, std::move(item)}).first->second);

        return id;
    }

    void add_item(line&& item)
    {
        m_lines.push_back(std::move(item));
    }

    void write_svg(std::ostream& stream) const
    {
        jg::svg_writer svg{stream, m_size};
        svg.write_background();

        svg.write_comment("Grid");
        svg.write_grid(50);

        const jg::svg_paint_attributes default_paint{"#d7eff6", "black", "3"};

        constexpr float font_size = 25;
        jg::svg_text_attributes default_text;
        default_text.font.size = std::to_string(font_size);
        default_text.font.family = "sans-serif";
        default_text.font.weight = "bold";
        default_text.text_anchor = svg_text_anchor::middle;
        default_text.dominant_baseline = svg_dominant_baseline::middle;

        for (const auto& [_, item] : m_items)
        {
            const jg::rect bounds = std::visit([&](const auto& shape)
            {
                return shape.bounds();
            }, item);

            std::visit(jg::overload
            {
                [&](const jg::rectangle&)
                {
                    svg.write_rect({bounds.x, bounds.y, bounds.width, bounds.height}, default_paint);
                },
                [&](const jg::rhombus&)
                {
                    svg.write_rhombus({bounds.x, bounds.y, bounds.width, bounds.height}, default_paint);
                },
                [&](const jg::parallelogram&)
                {
                    svg.write_parallelogram({bounds.x, bounds.y, bounds.width, bounds.height}, default_paint);
                },
                [&](const jg::ellipse&)
                {
                    svg.write_ellipse({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, bounds.width / 2, bounds.height / 2, default_paint);
                },
                [&](const jg::circle&)
                {
                    const auto radius = std::min(bounds.width, bounds.height) / 2;
                    svg.write_circle({bounds.x + radius, bounds.y + radius}, radius, default_paint);
                }
            }, item);

            std::visit([&](const auto& shape)
            {
                svg.write_comment(shape.text());
                svg.write_text({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, shape.text(), default_text);
                
                for (const auto& anchor : shape.anchors())
                    svg.write_circle({anchor.x, anchor.y}, 5, {"red", "none", "1"});
            }, item);
        };

        const jg::svg_paint_attributes default_line{"none", "black", "3"};

        svg.write_comment("Arrows");

        for (const auto& line : m_lines)
        {
            const auto source_anchors = std::visit([&] (const auto& source)
            {
                return source.anchors();
            }, m_items.find(line.source_id)->second);

            const auto target_anchors = std::visit([&] (const auto& target)
            {
                return target.anchors();
            }, m_items.find(line.target_id)->second);

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

            jg::debug_verify(shortest_distance != std::numeric_limits<float>::max());

            svg.write_arrow({anchors.first.x, anchors.first.y},
                            {anchors.second.x, anchors.second.y},
                            default_line);
        }

        svg.write_title(m_title);
        svg.write_border();
    }

private:
    static item_id new_id()
    {
        static item_id id;
        return ++id;
    }

    std::string m_title;
    jg::size m_size;
    using shapes = std::variant<rectangle, rhombus, parallelogram, ellipse, circle>;
    std::map<item_id, shapes> m_items;
    std::vector<line> m_lines;
};

} // namespace jg

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
    jg::diagram diagram{"jg-diagram-sample"};

    const std::array item_ids
    {
        diagram.add_item(jg::rectangle    {{ 50, 100, 300, 100}, "Rectangle"}),
        diagram.add_item(jg::ellipse      {{500,  50, 300, 100}, "Ellipse"}),
        diagram.add_item(jg::rhombus      {{550, 500, 300, 100}, "Rhombus"}),
        diagram.add_item(jg::parallelogram{{ 50, 500, 400, 100}, "Parallelogram"}),
        diagram.add_item(jg::circle       {{200, 250, 150, 150}, "Circle"})
    };

    diagram.add_item(jg::line{item_ids[0], item_ids[1], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{item_ids[1], item_ids[2], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{item_ids[2], item_ids[3], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{item_ids[3], item_ids[4], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{item_ids[4], item_ids[0], jg::line_kind::filled_arrow});
    
    diagram.write_svg(std::cout);
}
