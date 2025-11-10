#pragma once
#include <string>

class Component {
public:
  std::string component_type;
  virtual ~Component() {};
};
