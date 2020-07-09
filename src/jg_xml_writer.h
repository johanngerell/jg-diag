#include <iostream>
#include <string>
#include <string_view>

namespace jg
{

class xml_writer final
{
public:
    static xml_writer root_element(std::ostream& stream, std::string name)
    {
        return {stream, std::move(name)};
    }

    static xml_writer child_element(xml_writer& parent, std::string name)
    {
        return {parent, std::move(name)};
    }

    xml_writer(xml_writer&& other)
        : m_stream{other.m_stream}
        , m_name{std::move(other.m_name)}
        , m_is_parent{other.m_is_parent}
    {
        other.m_stream = nullptr;
        other.m_is_parent = false;
    }

    xml_writer& operator=(xml_writer&& other)
    {
        m_stream = other.m_stream;
        m_name = std::move(other.m_name);
        m_is_parent = other.m_is_parent;
        other.m_stream = nullptr;
        other.m_is_parent = false;
    }

    ~xml_writer()
    {
        if (m_stream)
        {
            if (m_is_parent)
                *m_stream << "</" << m_name << ">";
            else
                *m_stream << " />";
            
            *m_stream << "\n";
        }
    }

    void write_attribute(std::string_view name, size_t value)
    {
        *m_stream << ' ' << name << "=\"" << value << "\"";
    }

    void write_attribute(std::string_view name, std::string_view value)
    {
        *m_stream << ' ' << name << "=\"" << value << "\"";
    }

    void write_text(std::string_view text)
    {
        if (!m_is_parent)
        {
            m_is_parent = true;
            *m_stream << ">";
        }

        *m_stream << text;
    }

private:
    xml_writer(std::ostream& stream, std::string name)
        : m_stream{&stream}
        , m_name{std::move(name)}
    {
        *m_stream << "<" << m_name;
    }

    xml_writer(xml_writer& parent, std::string name)
        : m_stream{parent.m_stream}
        , m_name{std::move(name)}
    {
        if (!parent.m_is_parent)
        {
            parent.m_is_parent = true;
            *m_stream << ">\n";
        }

        *m_stream << "<" << m_name;
    }

    std::ostream* m_stream{};
    std::string m_name;
    bool m_is_parent{};
};

} // namespace jg
