#include <string>
#include <vector>

namespace HelloImGui
{
    namespace FunctionalUtils
    {
        std::vector<std::string> split(const std::string& s, const std::string& token);
        std::vector<std::string> split_lines(const std::string& s);

        std::string read_text_file_or_empty(const std::string& path);
        void write_text_file(const std::string& path, const std::string& content);

        std::string lower_string(const std::string& s);

        template<typename T>
        bool vector_contains(const std::vector<T>& xs, const T& value)
        {
            for (const auto& v : xs)
                if (v == value)
                    return true;
            return false;
        }
    }
}