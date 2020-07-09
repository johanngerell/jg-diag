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

    jg::svg_text_attributes default_text;
    default_text.font_size = "25";
    default_text.font_weight = "bold";
    svg.write_text({75, 75}, default_text, "SomeName");
    svg.write_border();
}