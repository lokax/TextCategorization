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
    if (argc < 4) {
        return -1;
    }

    std::string text_file(argv[2]);
    std::string out_text_file(argv[3]);
    std::string config_file_name(argv[1]);
    FileReader config_file(config_file_name, 0);
    std::vector<std::string> in_file_names;
    std::vector<std::string> out_file_names;
    std::vector<std::string> names;
    std::string line;
    while (true) {
        line.clear();
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

    std::vector<std::unordered_map<std::string, size_t>> pos_map_vec;
    for (size_t index = 0; index < out_file_names.size(); index++) {
        std::unordered_map<std::string, size_t> map;
        map.reserve(400);
        pos_map_vec.push_back(std::move(map));
    }

    for (size_t index = 0; index < out_file_names.size(); ++index) {
        FileReader profile_reader(out_file_names[index], index);
        auto& pos_map = pos_map_vec[index];
        while (true) {
            // auto line =
            line.clear();
            profile_reader.ReadLine(line);
            if (line.empty()) {
                break;
            }

            auto pos = line.find(' ');
            assert(pos != std::string::npos);
            auto str = line.substr(0, pos);
            pos_map[str] = pos_map.size();
        }
    }

    // Verify names
    // VerifyNames(in_file_names, out_file_names, names);

    // auto start = std::chrono::system_clock::now();

    FileReader text_reader(text_file, 0);
    std::filesystem::path path = out_text_file;
    // get dir path
    auto out_dir_path = path.remove_filename();
    if (!std::filesystem::exists(out_dir_path)) {
        // create dir
        std::filesystem::create_directories(out_dir_path);
    }

    FileWriter text_writer(out_text_file, 0);

    std::vector<ssize_t> distiance;
    distiance.resize(pos_map_vec.size(), 0);

    NGramParser<5> ngrams_parser(0);
    std::string text;
    while (true) {
        text.clear();
        // auto text =
        text_reader.ReadLine(text);
        if (text.empty()) {
            break;
        }

        auto ngrams_vec = ngrams_parser.ParseText(text);
        distiance.clear();
        distiance.resize(pos_map_vec.size(), 0);
        
        for (size_t midx = 0; midx < pos_map_vec.size(); ++midx) {
            auto& pos_map = pos_map_vec[midx];
            for (size_t index = 0; index < ngrams_vec.size(); ++index) {
                auto iter = pos_map.find(ngrams_vec[index].str);
                if (iter != pos_map.end()) {
                    distiance[midx] += std::abs(((ssize_t)iter->second - (ssize_t)index));
                } else {
                    distiance[midx] += pos_map.size();
                }
            }
        }
        

        size_t min_index = std::min_element(distiance.begin(), distiance.end()) - distiance.begin();

        text_writer.WriteLine(names[min_index]);
    }

    /*
    auto end = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "time : " << (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den << "秒"
              << "\n";
    */
    return 0;
}