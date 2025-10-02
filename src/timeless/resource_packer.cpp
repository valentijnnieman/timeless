#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    std::string assets_folder = "Assets";
    if (argc > 1) {
        assets_folder = argv[1];
    }
    std::vector<std::string> resource_files;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(assets_folder)) {
        if (entry.is_regular_file()) {
            resource_files.push_back(entry.path().string());
        }
    }

    std::ofstream out("assets.bin", std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open assets.bin for writing\n";
        return 1;
    }

    int file_count = resource_files.size();
    out.write(reinterpret_cast<const char*>(&file_count), sizeof(file_count));

    for (const auto& path : resource_files) {
        std::ifstream in(path, std::ios::binary);
        if (!in) {
            std::cerr << "Missing: " << path << "\n";
            continue;
        }
        std::vector<char> data((std::istreambuf_iterator<char>(in)), {});
        int name_len = path.size();
        int data_len = data.size();
        out.write(reinterpret_cast<const char*>(&name_len), sizeof(name_len));
        out.write(path.data(), name_len);
        out.write(reinterpret_cast<const char*>(&data_len), sizeof(data_len));
        out.write(data.data(), data_len);
    }

    out.close();
    std::cout << "Packed " << file_count << " files into assets.bin\n";
    return 0;
}
