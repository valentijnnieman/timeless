#pragma once
#include "timeless/timer.hpp"
#include "timeless/systems/system.hpp"

class NpcAiSystem : public System {
private:
  Timer timer = Timer(1.0f);
  bool forward = true;
  bool playing_solution = false;
  int start_hour = 7;
  int max_time = 200;

  Entity index_text;

public:
  float speed = 1.0f;
  bool running = false;
  bool viewing_ui = false; // separate for when viewing UI etc

  int main_index = 0;
  int solution_index = 0;

  void attach_text_component(ComponentManager &cm, Entity text);
  void change_timer(float new_time);
  void play_forward(ComponentManager &cm);
  void play_backward(ComponentManager &cm);
  void pause(ComponentManager &cm);
  void resume(ComponentManager &cm);
  std::string index_to_time();
  void update_text(ComponentManager &cm);
  void update(ComponentManager &cm);
};
