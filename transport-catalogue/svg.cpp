#include "svg.h"
using namespace std;
namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);
        context.out << endl;
    }

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
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point) {
        coordinates_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const Point& itemPoint : coordinates_) {
            if (!first) {
                out << " ";
            }
            out << itemPoint.x << "," << itemPoint.y;
            first = false;
        }
        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(string data) {
        data_ = PrepareText(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_;
        if (!font_family_.empty()) {
            out << "\" "sv;
            out << "font-family=\""sv << font_family_;
        }
        if (!font_weight_.empty()) {
            out << "\" "sv;
            out << "font-weight=\""sv << font_weight_;
        }
        out << "\""sv;
        out << ">"sv;
        out << data_;
        out << "</text>";
    }

    std::string Text::PrepareText(string data) {
        std::unordered_map<char, string> vocablary = {
            {'&', "&amp;"},
            {'"', "&quot;"},
            {'<', "&lt;"},
            {'>', "&gt;"},
            {'\'', "&apos;"}
        };
        string excCh = "aqlg";
        string resultString;
        resultString.reserve(vocablary.size());
        for (auto it = data.begin(); it < data.end(); ++it) {
            try {
                string str_ind = vocablary.at(*it);
                if (*it == '&' && excCh.find((*it + 1))) {
                    resultString.push_back(*it);
                }
                else {
                    for (char insertChar : vocablary.at(*it)) {
                        resultString.push_back(insertChar);
                    }
                }
            }
            catch (...) {
                resultString.push_back(*it);
            }
        }
        return resultString;
    }


    void Document::AddPtr(unique_ptr<Object>&& obj) {
        objects_ptr_.push_back(move(obj));
    }

    void Document::Render(ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << endl;
        for (const unique_ptr<Object>& itemObject : objects_ptr_) {
            RenderContext ctl(out);
            itemObject.get()->Render(ctl);
        }
        out << "</svg>"sv;
    }
}