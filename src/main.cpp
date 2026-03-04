#include "storage.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
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
  std::string add_habit_content;
  ftxui::InputOption add_habit_option;
  add_habit_option.on_enter = [&add_habit_content]{
    time_habits.push_back({0, add_habit_content});
    // save(time_habits);
    add_habit_content = "";
  };
  ftxui::Component add_habit = ftxui::Input(&add_habit_content, "add habit", add_habit_option) | ftxui::borderRounded;

  ftxui::Component habits_list = ftxui::Container::Vertical({});
  std::function<void()> refresh_habits_list = [&]{
    habits_list->Detach();

    for(int h = 0; h < time_habits.size(); h++){
      ftxui::Component play = ftxui::Button("Play", []{
      });
      ftxui::Component remove = ftxui::Button("Delete", [&]{
        time_habits.erase(time_habits.begin() + h);
        refresh_habits_list();
      });
      ftxui::Component container = ftxui::Container::Horizontal({play, remove});

      ftxui::Component visual = ftxui::Renderer(container, [&]{
        return ftxui::border(ftxui::hbox({
          play->Render(),
          ftxui::text(" " + std::to_string(time_habits[h].first) + " "),
          ftxui::text(time_habits[h].second),
          ftxui::filler(),
          remove->Render(),
        }));

        habits_list->Add(visual);
      });
    }
  };

  ftxui::Component habit_menu_container = ftxui::Container::Vertical({
    habits_list,
    add_habit,
  });
  ftxui::Component habit_menu = ftxui::Renderer(habit_menu_container, [habits_list, add_habit]{
    return ftxui::vbox({
      habits_list->Render(),
      ftxui::filler(),
      add_habit->Render()
    });
  });

  ftxui::Component main_content = ftxui::Container::Tab({
    habit_menu,
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

  refresh_habit_list();
  screen.Loop(final_app);
}
