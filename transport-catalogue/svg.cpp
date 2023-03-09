#include "svg.h"
using namespace std;
namespace svg{

ostream& operator<<(ostream& out, const StrokeLineCap& line_cap){
   switch (line_cap){
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    ostream& operator<<(ostream& out, const StrokeLineJoin& line_join){
        switch (line_join){
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const{
        context.RenderIndent();
        RenderObject(context);
    }

    Circle& Circle::SetCenter(Point center){
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius){
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const{
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    Polyline& Polyline::AddPoint(Point point){
        points_.emplace_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const{
        auto& out = context.out;
        bool print_space = false;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points_.size(); ++i){
            if (print_space){
                out << " "sv;
            }
            else{
                print_space = true;
            }
            out << points_[i].x << ","sv << points_[i].y;
        }
        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }


    Text& Text::SetPosition(Point pos){
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset){
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size){
        size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(string font_family){
        font_family_ = move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(string font_weight){
        font_weight_ = move(font_weight);
        return *this;
    }

    Text& Text::SetData(string data){
        data_ = move(data);
        return *this;
    }

    static unordered_map<char, string> html_symbols = {
        {'&', std::string("&amp;")},
        {'\"', std::string("&quot;")},
        {'\'', std::string("&apos;")},
        {'<', std::string("&lt;")},
        {'>', std::string("&gt;")}
    };

   string Text::Encode(std::string data) const{
        string substring;
        string replace_to;
        size_t pos = 0;
        for (const auto& [key, value] : html_symbols)
        {
            pos = data.find(key);
            while (pos != string::npos)
            {
                data.replace(pos, sizeof(key), value);
                pos = data.find(substring, pos + value.size());
            }
        }
        return data;
    }

    void Text::RenderObject(const RenderContext& context) const{
        auto& out = context.out;
        out << "<text"sv;
        out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\"" << size_ << "\""sv;
        if (font_family_.size() > 0){
            out << " font-family=\"" << font_family_ << "\""sv;
        }
        if (font_weight_.size() > 0){
            out << " font-weight=\"" << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">"sv;
        out << Encode(data_);
        out << "</text>"sv;
    }

    void Document::Render(ostream& out) const{ 
        RenderContext context_ = RenderContext(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << endl;
        context_.indent += context_.indent_step;
        for (size_t i = 0; i < objects_.size(); ++i)
        {
            objects_[i]->Render(context_);
            out << std::endl;
        }
        context_.indent -= context_.indent_step;
        out << "</svg>"sv;
    }
} 