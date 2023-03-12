#include <stdexcept>

#include "svg.h"

namespace svg {

    using namespace std::literals;

    // ---------- Point ------------------

    Point::Point(double x, double y)
        : x(x)
        , y(y) {
    }

    // ---------- RenderContext ------------------

    RenderContext::RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Color ------------------

    Rgb::Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red(red)
        , green(green)
        , blue(blue) {
    }

    Rgba::Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : red(red)
        , green(green)
        , blue(blue)
        , opacity(opacity) {
    }

    struct ColorPrinter {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none"sv;
        }

        void operator()(const std::string& color) const {
            out << color;
        }

        void operator()(const Rgb& color) const {
            out << "rgb("sv
                << static_cast<int>(color.red) << ','
                << static_cast<int>(color.green) << ','
                << static_cast<int>(color.blue)
                << ')';
        }

        void operator()(const Rgba& color) const {
            out << "rgba("sv
                << static_cast<int>(color.red) << ','
                << static_cast<int>(color.green) << ','
                << static_cast<int>(color.blue) << ','
                << color.opacity
                << ')';
        }
    };

    std::ostream& operator<<(std::ostream& out, const Color& color) {
        ColorPrinter printer{ out };
        std::visit(printer, color);
        return out;
    }

    // ---------- Enums ------------------

    std::ostream& operator<<(std::ostream& out, StrokeLineCap linecap) {
        using namespace std::literals;
        std::string linecap_str;

        switch (linecap) {
        case StrokeLineCap::BUTT:
            linecap_str = "butt"s;
            break;
        case StrokeLineCap::ROUND:
            linecap_str = "round"s;
            break;
        case StrokeLineCap::SQUARE:
            linecap_str = "square"s;
            break;
        default:
            throw std::invalid_argument("invalid linecap");
        }

        out << linecap_str;
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin linejoin) {
        using namespace std::literals;
        std::string linejoin_str;

        switch (linejoin) {
        case svg::StrokeLineJoin::ARCS:
            linejoin_str = "arcs"s;
            break;
        case svg::StrokeLineJoin::BEVEL:
            linejoin_str = "bevel"s;
            break;
        case svg::StrokeLineJoin::MITER:
            linejoin_str = "miter"s;
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            linejoin_str = "miter-clip"s;
            break;
        case svg::StrokeLineJoin::ROUND:
            linejoin_str = "round"s;
            break;
        default:
            throw std::invalid_argument("invalid linejoin");
        }

        out << linejoin_str;
        return out;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle"sv;
        out << " cx=\""sv << center_.x << "\""sv;
        out << " cy=\""sv << center_.y << "\""sv;
        out << " r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const auto& point : points_) {
            if (!first) {
                out << ' ';
            }
            out << point.x << ',' << point.y;
            first = false;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text"sv;
        out << " x=\""sv << pos_.x << "\""sv;
        out << " y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\""sv;
        out << " dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\""sv;
        if (font_family_) {
            out << " font-family=\""sv << *font_family_ << "\""sv;
        }
        if (font_weight_) {
            out << " font-weight=\""sv << *font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">";

        out << EscapedString(data_);

        out << "</text>"sv;
    }

    std::string Text::EscapedString(const std::string& str) {
        std::string escaped_str;
        for (char c : str) {
            switch (c) {
            case '"':
                escaped_str += "&quot;"s;
                break;
            case '\'':
                escaped_str += "&apos;"s;
                break;
            case '<':
                escaped_str += "&lt;"s;
                break;
            case '>':
                escaped_str += "&gt;"s;
                break;
            case '&':
                escaped_str += "&amp;"s;
                break;
            default:
                escaped_str.push_back(c);
            }
        }
        return escaped_str;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext context(out, 2, 2);
        for (const auto& obj : objects_) {
            obj->Render(context);
        }

        out << "</svg>"sv;
    }

}  // namespace svg
