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
  HabitsMenue,
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
  ScreenStatus current_screen = ScreenStatus::HabitsMenue;

  std::vector<std::string> bbar_entries = {"Habits", "Breaks"};
  int bbar_selector = 0;
  int prev_bbar_selector = bbar_selector;
  ftxui::MenuOption bbar_option;
  ftxui::Component bbar = ftxui::Toggle(&bbar_entries, &bbar_selector) | ftxui::border;

  ftxui::Component tbar = ftxui::Renderer([]{
    return ftxui::vbox(ftxui::text("Work done today: " + workdonetoday()) | ftxui::center);
  });

  std::string add_habits_content;
  ftxui::InputOption add_habits_option;
  add_habits_option.on_enter = [&add_habits_content]{
    time_habits.push_back({0, add_habits_content});
    // save(time_habits);
    add_habits_content = "";
  };
  ftxui::Component add_habits = ftxui::Input(&add_habits_content, "add habit", add_habits_option) | ftxui::borderRounded;

  ftxui::Component habits_menu = ftxui::Container::Vertical({
    tbar,
    add_habits,
    bbar,
  });

  ftxui::Component renderer = ftxui::Renderer(habits_menu, [&]{
    if(bbar_selector != prev_bbar_selector){
      prev_bbar_selector = bbar_selector;
      if(bbar_selector == 0){
        current_screen = ScreenStatus::HabitsMenue;
      }
      else if(bbar_selector == 1){ 
        current_screen = ScreenStatus::BreaksTimer;
      }
    }

    if(current_screen == ScreenStatus::HabitsMenue){
      return ftxui::vbox({
        tbar->Render(),
        ftxui::filler(),
        add_habits->Render(),
        bbar->Render(),
      });
    }
    else if(current_screen == ScreenStatus::BreaksTimer){
      return ftxui::vbox({
        tbar->Render(),
        ftxui::filler(),
        bbar->Render(),
      });
    }
    else if(current_screen == ScreenStatus::HabitsTimer){
      return ftxui::text("habits timer");
    }

    return ftxui::text("error");
  });

  screen.Loop(renderer);
}
