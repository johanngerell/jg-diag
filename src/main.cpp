#include "jg_svg_writer.h"
#include <variant>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

namespace jg
{
    class diagram;
}

std::ostream& operator<<(std::ostream&, const jg::diagram&);

namespace jg
{

class rectangle final
{
public:
    rectangle(jg::rect bounds, std::string_view text)
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

    std::array<jg::point, 4> anchors() const
    {
        return
        {
            jg::point{m_bounds.x                     , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height}
        };
    }

private:
    jg::rect m_bounds;
    std::string m_text;
};

class rhombus final
{
public:
    rhombus(jg::rect bounds, std::string_view text)
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

    std::array<jg::point, 4> anchors() const
    {
        return
        {
            jg::point{m_bounds.x                     , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height}
        };
    }

private:
    jg::rect m_bounds;
    std::string m_text;
};

class parallelogram final
{
public:
    parallelogram(jg::rect bounds, std::string_view text)
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

    std::array<jg::point, 4> anchors() const
    {
        return
        {
            jg::point{m_bounds.x + m_bounds.height / 2, m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y},
            jg::point{m_bounds.x + m_bounds.width - m_bounds.height / 2, m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height}
        };
    }

private:
    jg::rect m_bounds;
    std::string m_text;
};

class ellipse final
{
public:
    ellipse(jg::rect bounds, std::string_view text)
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

    std::array<jg::point, 4> anchors() const
    {
        return
        {
            jg::point{m_bounds.x                     , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width    , m_bounds.y + m_bounds.height / 2},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y},
            jg::point{m_bounds.x + m_bounds.width / 2, m_bounds.y + m_bounds.height}
        };
    }

private:
    jg::rect m_bounds;
    std::string m_text;
};

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

class diagram final
{
public:
    template <typename T>
    item_id add_item(T&& item)
    {
        const auto id = new_id();
        
        std::visit([&](const auto& i)
        {
            const auto& bounds = i.bounds();

            if (bounds.x + bounds.width > m_extent.width - 50)
                m_extent.width = bounds.x + bounds.width + 50;

            if (bounds.y + bounds.height > m_extent.height - 50)
                m_extent.height = bounds.y + bounds.height + 50;

        }, m_items.insert({id, std::move(item)}).first->second);

        return id;
    }

    item_id add_item(line&& item)
    {
        m_lines.push_back(std::move(item));
        return 0;
    }

private:
    static item_id new_id()
    {
        static item_id id;
        return ++id;
    }

    friend std::ostream& ::operator<<(std::ostream&, const diagram&);

    jg::size m_extent;
    std::map<item_id, std::variant<rectangle, rhombus, parallelogram, ellipse>> m_items;
    std::vector<line> m_lines;
};

// https://www.bfilipek.com/2018/06/variant.html#overload
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

} // namespace jg

std::ostream& operator<<(std::ostream& stream, const jg::diagram& diagram)
{
    jg::svg_writer svg{stream, diagram.m_extent};
    svg.write_background();
    svg.write_grid(50);

    jg::svg_rect_attributes default_rect;
    default_rect.fill = "#d7eff6";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";

    constexpr float font_size = 25;
    jg::svg_text_attributes default_text;
    default_text.font_size = std::to_string(font_size);
    default_text.font_weight = "bold";

    jg::svg_circle_attributes default_circle;
    default_circle.fill = "red";

    for (const auto& [_, item] : diagram.m_items)
    {
        const jg::rect bounds = std::visit([&](const auto& i) { return i.bounds(); }, item);

        std::visit(jg::overload
        {
            [&](const jg::rectangle&)
            {
                svg.write_rect({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            },
            [&](const jg::rhombus&)
            {
                svg.write_rhombus({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            },
            [&](const jg::parallelogram&)
            {
                svg.write_parallelogram({bounds.x, bounds.y, bounds.width, bounds.height}, default_rect);
            },
            [&](const jg::ellipse&)
            {
                svg.write_ellipse({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, bounds.width / 2, bounds.height / 2, default_rect);
            }
        }, item);

        std::visit([&](const auto& i)
        {
            svg.write_text({bounds.x + bounds.width / 2, bounds.y + bounds.height / 2}, default_text, i.text());
            for (const auto& anchor : i.anchors())
                svg.write_circle({anchor.x, anchor.y}, 5, default_circle);
        }, item);
    };

    jg::svg_line_attributes default_line;
    default_line.stroke_width = "3";

    for (const auto& line : diagram.m_lines)
    {
        std::array<jg::point, 4> source_anchors;
        std::visit([&] (const auto& source)
        {
            source_anchors = source.anchors();
        }, diagram.m_items.find(line.source_id)->second);

        std::array<jg::point, 4> target_anchors;
        std::visit([&] (const auto& target)
        {
            target_anchors = target.anchors();
        }, diagram.m_items.find(line.target_id)->second);

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
    jg::diagram diagram;

    const std::array items
    {
        diagram.add_item(jg::rectangle    {{ 50, 100, 300, 100}, "Rectangle"}),
        diagram.add_item(jg::ellipse      {{500,  50, 300, 100}, "Ellipse"}),
        diagram.add_item(jg::rhombus      {{550, 400, 300, 100}, "Rhombus"}),
        diagram.add_item(jg::parallelogram{{100, 300, 400, 100}, "Parallelogram"})
    };

    diagram.add_item(jg::line{items[0], items[1], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{items[1], items[2], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{items[2], items[3], jg::line_kind::filled_arrow});
    diagram.add_item(jg::line{items[3], items[0], jg::line_kind::filled_arrow});
    
    std::cout << diagram;
}
