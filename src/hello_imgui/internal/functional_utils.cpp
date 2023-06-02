#include "hello_imgui/internal/functional_utils.h"

#include <string>
#include <fstream>

namespace HelloImGui
{
    namespace FunctionalUtils
    {
        inline std::vector<std::string> split(const std::string& s, const std::string& token)
        {
            std::string str =s;
            std::vector<std::string>result;
            while(str.size())
            {
                size_t index = str.find(token);
                if(index != std::string::npos)
                {
                    result.push_back(str.substr(0,index));
                    str = str.substr(index + token.size());
                    if(str.size()==0)result.push_back(str);
                }
                else
                {
                    result.push_back(str);
                    str = "";
                }
            }
            return result;
        }

        std::vector<std::string> split_lines(const std::string& s)
        {
            return split(s, "\n");
        }

        std::string read_text_file_or_empty(const std::string& path)
        {
            constexpr auto read_size = std::size_t(4096);
            auto stream = std::ifstream(path);
            stream.exceptions(std::ios_base::badbit);

            if (! stream)
                return "";

            auto out = std::string();
            auto buf = std::string(read_size, '\0');
            while (stream.read(& buf[0], read_size))
                out.append(buf, 0, stream.gcount());
            out.append(buf, 0, stream.gcount());
            return out;
        }

        void write_text_file(const std::string& path, const std::string& content)
        {
            auto stream = std::ofstream(path);
            if (stream.good())
                stream << content;
        }

        std::string lower_string(const std::string& s)
        {
            std::string r;
            for (auto c: s)
                c += std::tolower(c);
            return r;
        }


    }
}