#include <getopt.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Option {
    char short_name;
    std::string long_name;
    std::string arg_name;
    std::string help;
    std::string default_value;
    bool required;
};

std::unordered_map<std::string, std::vector<std::string>> getopt(std::string name, int argc, char * const argv[], std::vector<Option> options)
{
    std::string optstring = "h";
    std::vector<struct option> long_options = {{"help", 0, 0, -2}};
    std::unordered_set<uint8_t> required;
    for (int i = 0; i < (int)options.size(); i ++) {
        auto & opt = options[i];
        optstring += opt.short_name;
        if (opt.arg_name.size()) {
            optstring += ':';
        }
        if (opt.required) {
            required.insert(i);
        }
        long_options.push_back({opt.long_name.c_str(), opt.arg_name.size() ? 1 : 0, 0, i});
    }
    long_options.push_back({0, 0, 0, 0});

    int opt;
    optind = 1;
    std::unordered_map<std::string, std::vector<std::string>> args;
    while (true) {
        opt = getopt_long_only(argc, argv, optstring.c_str(), long_options.data(), NULL);
        if (opt == -2) { required.insert(opt); break; }
        if (opt == -1) { break; }
        if (optarg) {
            args[options[opt].long_name].push_back(optarg);
        } else {
            args[options[opt].long_name].push_back("");
        }
        required.erase(opt);
    }
    if (args.count("help") || required.size()) {
        std::cout << "Usage: " << name << " todoshowargs";
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        size_t max_longname_len = 0;
        for (auto & opt : options) {
            size_t len = opt.long_name.size();
            if (opt.arg_name.size()) {
                len += opt.arg_name.size() + 1;
            }
            if (len > max_longname_len) {
                max_longname_len = len;
            }
        }
        for (auto & opt : options) {
            size_t len = opt.long_name.size();
            std::cout << "  ";
            if (opt.short_name) {
                std::cout << "-" << opt.short_name << ", ";
            } else {
                std::cout << "    ";
            }
            if (opt.long_name.size()) {
                std::cout << "--" << opt.long_name;
                if (opt.arg_name.size()) {
                    std::cout << "=" << opt.arg_name;
                    len += 1 + opt.arg_name.size();
                }
            } else {
                if (opt.arg_name.size()) {
                    std::cout << "i " << opt.arg_name;
                    len += opt.arg_name.size();
                } else {
                    std::cout << "  ";
                }
            }
            for (; len < max_longname_len; len ++) {
                std::cout << " ";
            }
            std::cout << " " << opt.help << std::endl;
        }
        exit(0);
    }
    for (auto & opt : options) {
        if (!args.count(opt.long_name)) {
            args[opt.long_name].push_back(opt.default_value);
        }
    }
    return args;
}
