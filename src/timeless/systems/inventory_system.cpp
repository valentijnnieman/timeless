#include "timeless/systems/inventory_system.hpp"

Entity InventorySystem::get_current() {
    try {
        if (registered_entities.size() > static_cast<size_t>(selected)) {
            return registered_entities[selected];
        } else {
            throw std::out_of_range("could not find inventory item");
        }
    } catch (const std::out_of_range &e) {
        std::cerr << "Exception at " << e.what() << std::endl;
        return 0;
    }
}
