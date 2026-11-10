#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>

class ResourceLoader {
public:
    std::unordered_map<std::string, std::vector<uint8_t>> assets;

    bool load(const std::string& archive_path);

    const std::vector<uint8_t>& get(const std::string& name) const;

    // Returns file contents as a string.
    // WASM: reads directly from disk. Native: reads from loaded assets.bin.
    std::string get_string(const std::string& name) const;
};
