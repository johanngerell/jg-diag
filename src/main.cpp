#include "jg_svg_writer.h"

int main()
{
    jg::svg_writer svg{std::cout, {1024, 768}};
    svg.write_background();
    svg.write_grid(50);

    jg::svg_rect_attributes default_rect;
    default_rect.fill = "whitesmoke";
    default_rect.stroke = "black";
    default_rect.stroke_width = "3";
    svg.write_rect({50, 50, 300, 50}, default_rect);
    svg.write_rect({500, 50, 300, 50}, default_rect);
    svg.write_rect({500, 250, 300, 50}, default_rect);
    svg.write_rect({50, 250, 300, 50}, default_rect);

    jg::svg_line_attributes default_line;
    default_line.stroke_width = "3";
    svg.write_arrow({350, 75}, {500, 75}, default_line);
    svg.write_arrow({200, 250}, {200, 100}, default_line);
    svg.write_arrow({500, 275}, {350, 275}, default_line);
    svg.write_arrow({650, 100}, {650, 250}, default_line);

    jg::svg_text_attributes default_text;
    default_text.font_size = "25";
    default_text.font_weight = "bold";
    svg.write_text({75, 75}, default_text, "Box 1");
    svg.write_text({525, 75}, default_text, "Box 2");
    svg.write_text({525, 275}, default_text, "Box 3");
    svg.write_text({75, 275}, default_text, "Box 4");
    svg.write_border();
}