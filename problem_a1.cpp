#include "ngrams.h"

// DEBUG
static void VerifyNames(std::vector<std::string>& in_file_names, std::vector<std::string>& out_file_names, std::vector<std::string>& names)
{
    for (size_t index = 0; index < in_file_names.size(); ++index) {
        std::cout << "in_file_name = " << in_file_names[index] << "\n";
        std::cout << "out_file_name = " << out_file_names[index] << "\n";
        std::cout << "name = " << names[index] << "\n";
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return -1;
    }

    std::string config_file_name(argv[1]);
    FileReader config_file(config_file_name, 0);
    std::vector<std::string> in_file_names;
    std::vector<std::string> out_file_names;
    std::vector<std::string> names;
    std::string line;
    while (true) {
        line.clear();
        // std::string line =
        config_file.ReadLine(line);
        // EOF
        if (line.empty()) {
            break;
        }
        // find first space
        size_t lpos = line.find(' ');
        if (lpos == std::string::npos) {
            std::cout << "invalid config\n";
            return -1;
        }

        in_file_names.push_back(line.substr(0, lpos));

        // skip space
        while (lpos < line.size() && line[lpos] == ' ') {
            lpos++;
        }

        // find next space
        size_t rpos = line.find(' ', lpos);
        if (rpos == std::string::npos) {
            std::cout << "invalid config\n";
            return -1;
        }

        out_file_names.push_back(line.substr(lpos, rpos - lpos));
        std::filesystem::path path = out_file_names.back();
        // get dir path
        auto dir_path = path.remove_filename();
        if (!std::filesystem::exists(dir_path)) {
            // create dir
            std::filesystem::create_directories(dir_path);
        }

        // skip space
        while (rpos < line.size() && line[rpos] == ' ') {
            rpos++;
        }

        if (rpos == line.size()) {
            std::cout << "invalid config\n";
            return -1;
        }

        names.push_back(line.substr(rpos));
    }

    // Verify names
    // VerifyNames(in_file_names, out_file_names, names);

    // auto start = std::chrono::system_clock::now();

    for (size_t index = 0; index < in_file_names.size(); ++index) {
        NGramParser<5> ngram_parser(in_file_names[index], out_file_names[index], index);
        ngram_parser.Parse();
        ngram_parser.Generate();
    }

    /*
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time : " << (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒"
              << "\n";
    */
    return 0;
}