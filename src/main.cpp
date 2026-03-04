#include "storage.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>
#include <utility>

std::vector<std::pair<int, std::string>> time_habits;

enum class ScreenStatus{
  HabitsMenu,
  HabitsTimer,
  BreaksTimer,
};

std::string workdonetoday(){
  int out = 0;
  for(std::pair<int, std::string> i : time_habits){
    out += i.first;
  }
  return std::to_string(out) + 'h';
}

int main(){
  load(time_habits);

  auto screen = ftxui::ScreenInteractive::Fullscreen();

  int current_screen = 0; 

  // bottom bar -> for changin habit and break mode
  std::vector<std::string> bbar_entries = {"Habits", "Breaks"};
  int bbar_selector = current_screen; // 0 or Habits menu
  ftxui::MenuOption bbar_option;
  ftxui::Component bbar = ftxui::Toggle(&bbar_entries, &bbar_selector) | ftxui::border;

  // top bar -> for showing how much progress use make
  ftxui::Component tbar = ftxui::Renderer([]{
    return ftxui::vbox(ftxui::text("Work done today: " + workdonetoday()) | ftxui::center);
  });

  // for adding habits
  std::string add_habits_content;
  ftxui::InputOption add_habits_option;
  add_habits_option.on_enter = [&add_habits_content]{
    time_habits.push_back({0, add_habits_content});
    // save(time_habits);
    add_habits_content = "";
  };
  ftxui::Component add_habits = ftxui::Input(&add_habits_content, "add habit", add_habits_option) | ftxui::borderRounded;

  ftxui::Component habit_start = ftxui::Button("Play", [&]{
    current_screen = static_cast<int>(ScreenStatus::HabitsTimer);
  });
  ftxui::Component habit_delete = ftxui::Button("Delete", [&]{
    
  });
  ftxui::Component habit_box_component = ftxui::Container::Horizontal({
    habit_start,
    habit_delete,
  });
  auto habit_box = [&](int time, std::string& name, int idx){
    return ;
  };
  ftxui::Component habit_list = ftxui::Renderer([]{
    ftxui::Components content;
    for(int h = 0; h < time_habits.size(); h++){
      int time = time_habits[h].first; std::string name = time_habits[h].second;
      content.push_back(habits_content_box(time, name, h));
    }
  });

  ftxui::Component main_content = ftxui::Container::Tab({

  }, &current_screen);

  ftxui::Component main_app = ftxui::Renderer([&]{
    return ftxui::vbox({
      tbar->Render(),
      main_content->Render(),
      bbar->Render(),
    });
  });

  ftxui::Component pop_up_component = ftxui::Container::Vertical()

  ftxui::Component final_app = ;

  screen.Loop(final_app);
}
