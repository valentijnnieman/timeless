#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

class ResourceLoader {
public:
    std::unordered_map<std::string, std::vector<uint8_t>> assets;

    bool load(const std::string& archive_path) {
        std::ifstream in(archive_path, std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open " << archive_path << "\n";
            return false;
        }
        uint32_t file_count;
        in.read(reinterpret_cast<char*>(&file_count), sizeof(file_count));
        for (uint32_t i = 0; i < file_count; ++i) {
            uint32_t name_len, data_len;
            in.read(reinterpret_cast<char*>(&name_len), sizeof(name_len));
            std::string name(name_len, '\0');
            in.read(&name[0], name_len);
            in.read(reinterpret_cast<char*>(&data_len), sizeof(data_len));
            std::vector<uint8_t> data(data_len);
            in.read(reinterpret_cast<char*>(data.data()), data_len);
            assets[name] = std::move(data);
        }
        return true;
    }

    const std::vector<uint8_t>& get(const std::string& name) const {
      return assets.at(name);
    }
};
