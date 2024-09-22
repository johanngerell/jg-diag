#pragma once

#include <iostream>
#include <string>

namespace jg
{

class xml_writer final
{
public:
    static xml_writer root_element(std::ostream& stream, std::string_view name)
    {
        return {stream, name};
    }

    static xml_writer child_element(xml_writer& parent, std::string_view name)
    {
        return {parent, name};
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

        return *this;
    }

    ~xml_writer()
    {
        if (!m_stream)
            return;

        if (m_is_parent)
            *m_stream << "</" << m_name << (m_is_comment ? "-->" : ">");
        else
            *m_stream << (m_is_comment ? " /-->" : " />");
        
        *m_stream << "\n";
    }

    template <typename T>
    void write_attribute(std::string_view name, const T& value)
    {
        *m_stream << ' ' << name << "=\"" << value << "\"";
    }

    void write_comment(std::string_view comment)
    {
        m_is_comment = true;
        *m_stream << "!--" << comment;
    }

    void write_text(std::string_view text)
    {
        if (!m_is_parent)
        {
            m_is_parent = true;
            *m_stream << (m_is_comment ? "-->" : ">");
        }

        *m_stream << text;
    }

private:
    xml_writer(std::ostream& stream, std::string_view name)
        : m_stream{&stream}
        , m_name{name}
    {
        *m_stream << "<" << m_name;
    }

    xml_writer(xml_writer& parent, std::string_view name)
        : m_stream{parent.m_stream}
        , m_name{name}
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
    bool m_is_comment{};
};

} // namespace jg
