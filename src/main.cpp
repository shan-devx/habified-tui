#include "storage.hpp"
#include <cstdlib>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <utility>
#include <atomic>

ftxui::Element giant_timer(char c);

std::vector<std::pair<int, std::string>> time_habits;

enum class ScreenStatus{
  HabitMenu,
  BreakTimer,
  HabitTimer,
};

std::string workdonetoday(){
  int second = 0;
  for(std::pair<int, std::string> i : time_habits){
    second += i.first;
  }
  int hour = second / (60*2); second = second - (hour * 60 * 2);
  int minute = second / 60; second = second - (minute * 60);

  return std::to_string(hour) + "h " + std::to_string(minute) + "m " + std::to_string(second) + "s ";
}

// start button, stop button, timer_count
ftxui::Component timer(ftxui::Component startb, ftxui::Component stopb, std::atomic<int> &timer_count){
  ftxui::Component timer_options = ftxui::Container::Horizontal({
    startb,
    stopb,
  });
  ftxui::Component out = ftxui::Renderer(timer_options, [&, stopb, startb]{
    int second = timer_count;
    int minute = second/60; second = second - (minute * 60);

    std::string minute_str = minute < 10 ? '0' + std::to_string(minute) : std::to_string(minute);
    std::string second_str = second < 10 ? '0' + std::to_string(second) : std::to_string(second);

    ftxui::Elements ascii_minute;
    for(char s : minute_str){
      ascii_minute.push_back(giant_timer(s));
    }

    ftxui::Elements ascii_second;
    for(char s : second_str){
      ascii_second.push_back(giant_timer(s));
    }

    ftxui::Element timer = ftxui::hbox({
      ftxui::hbox(ascii_minute),
      giant_timer(':'),
      ftxui::hbox(ascii_second),
    }) | ftxui::center;

    ftxui::Element options = ftxui::hbox({
      ftxui::filler(),
      startb->Render(),
      ftxui::filler(),
      stopb->Render(),
      ftxui::filler(),
    });

    return ftxui::vbox({
      timer,
      options,
    }) | ftxui::center;
  });

  return out;
}

int main(){
  load(time_habits);
  auto screen = ftxui::ScreenInteractive::Fullscreen();

  int current_screen = 0; 

  // bottom bar -> for changin habit and break mode
  std::vector<std::string> bbar_entries = {"Habits", "Breaks"};
  int bbar_selector = current_screen; // 0 or Habits menu
  int prev_bbar_selector = bbar_selector;
  ftxui::MenuOption bbar_option;
  bool show_bbar = true;
  ftxui::Component bbar = ftxui::Maybe(ftxui::Toggle(&bbar_entries, &bbar_selector) | ftxui::border, &show_bbar);

  // top bar -> for showing how much progress use make
  ftxui::Component tbar = ftxui::Renderer([]{
    return ftxui::vbox(ftxui::text("Work done: " + workdonetoday()) | ftxui::center);
  });

  // atomic is used to fix data race bug
  std::atomic<int> timer_count = 0; // in seconds
  std::atomic<bool> timer_running = false; 
  std::atomic<bool> tthread_alive = true;
  std::thread tm([&]{
    while(tthread_alive){
      if(timer_running){
        timer_count += 1;
        screen.PostEvent(ftxui::Event::Custom); // wake up call for ftxui to redraw on the screen
      }
      else{
        timer_count = 0;
        screen.PostEvent(ftxui::Event::Custom);
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  });

//------------------------------------------------break timer--------------------------------------------

  ftxui::Component start_break = ftxui::Button("Start", [&]{
    timer_running = true;
  });
  ftxui::Component stop_break = ftxui::Button("Clear", [&]{
    timer_running = false;
  });

  ftxui::Component break_timer_options = ftxui::Container::Horizontal({
    start_break,
    stop_break,
  });

  ftxui::Component break_timer = timer(start_break, stop_break, timer_count);

 //------------------------------------------------break timer--------------------------------------------
  
// ----------------------------------------------habit menu----------------------------------------------

  bool show_pop_up = false;
  ftxui::Component pop_up_yes = ftxui::Button("Stop break timer", [&]{
    timer_running = false;
    show_pop_up = false;
    show_bbar = false;
    current_screen = static_cast<int>(ScreenStatus::HabitTimer);
  });
  ftxui::Component pop_up_no = ftxui::Button("Discard action", [&]{
    show_pop_up = false;
  });
  ftxui::Component pop_up = ftxui::Container::Horizontal({
    pop_up_no,
    pop_up_yes,
  });

  int running_habit = 0;
  ftxui::Component habits_list = ftxui::Container::Vertical({});
  std::function<void()> refresh_habits_list = [&]{
    habits_list->DetachAllChildren();

    for(int h = 0; h < time_habits.size(); h++){
      ftxui::Component play = ftxui::Button("Play", [&, h]{
        if(timer_running){
          show_pop_up = true;
        }
        else{
          show_bbar = false;
          running_habit = h;
          current_screen = static_cast<int>(ScreenStatus::HabitTimer);
        }
      });
      ftxui::Component remove = ftxui::Button("Delete", [&, h]{ // h is needed by vlaue instead of reffrence
        time_habits.erase(time_habits.begin() + h);
        save(time_habits);
        refresh_habits_list();
      });
      ftxui::Component container = ftxui::Container::Horizontal({play, remove});

      ftxui::Component visual = ftxui::Renderer(container, [&, h, play, remove]{
        return ftxui::border(ftxui::hbox({
          play->Render(),
          ftxui::text(" " + workdonetoday()) | ftxui::center,
          ftxui::filler(),
          ftxui::text(time_habits[h].second) | ftxui::center,
          ftxui::filler(),
          remove->Render(),
        }));

      });
      habits_list->Add(visual);
    }
  };

  std::string add_habit_content;
  ftxui::InputOption add_habit_option;
  add_habit_option.on_enter = [&]{
    time_habits.push_back({0, add_habit_content});
    save(time_habits);
    add_habit_content = "";
    refresh_habits_list();
  };
  ftxui::Component add_habit = ftxui::Input(&add_habit_content, "add habit", add_habit_option) | ftxui::borderRounded;

  ftxui::Component habit_menu_container = ftxui::Container::Vertical({
    add_habit,
    habits_list,
  });
  ftxui::Component habit_menu = ftxui::Renderer(habit_menu_container, [habits_list, add_habit]{
    return ftxui::vbox({
      add_habit->Render(),
      habits_list->Render(),
    });
  });
 
  ftxui::Component start_habit = ftxui::Button("Start", [&]{
    timer_running = true;
  });

  ftxui::Component end_habit = ftxui::Button("End", [&]{
    time_habits[running_habit].first += timer_count;
    save(time_habits);
    current_screen = static_cast<int>(ScreenStatus::HabitMenu);
    show_bbar = true;
    timer_running = false;
  });
  
  ftxui::Component habit_timer = timer(start_habit, end_habit, timer_count);

//------------------------------------------------habit menu---------------------------------------------


  ftxui::Component main_content = ftxui::Container::Tab({
    habit_menu,
    break_timer,
    habit_timer,
  }, &current_screen);  // change it later to current_screen

  ftxui::Component app_content = ftxui::Container::Vertical({
    main_content,
    bbar,
  }); // tbar is not indcluded in component cause it is not interactive 


  ftxui::Component app = ftxui::Renderer(app_content, [&]{
    if(prev_bbar_selector != bbar_selector){
      current_screen = bbar_selector;
      prev_bbar_selector = bbar_selector;
    }
    return ftxui::vbox({
      tbar->Render(),
      main_content->Render() | ftxui::flex,
      bbar->Render(),
    });
  });

  pop_up = ftxui::Renderer(pop_up, [pop_up]{
    return ftxui::vbox({
      ftxui::text("Break timer is running, what do you want to do?"),
      ftxui::text(""),
      pop_up->Render() | ftxui::center,
    }) | ftxui::border;
  }) | ftxui::clear_under;
  app = ftxui::Modal(app, pop_up, &show_pop_up);

  refresh_habits_list();
  screen.Loop(app);
  
  tthread_alive = false;
  tm.join();
  return EXIT_SUCCESS;
}

ftxui::Element giant_timer(char c){
  std::vector<std::string> lines;

  switch (c) {
        case '0': lines = {
            "    ██████    ", 
            "  ███    ███  ", 
            " ███      ███ ", 
            " ███      ███ ", 
            " ███      ███ ", 
            " ███      ███ ", 
            " ███      ███ ", 
            " ███      ███ ", 
            "  ███    ███  ", 
            "    ██████    ",
            "              ",
        }; break;
        case '1': lines = {
            "      ███     ", 
            "    █████     ", 
            "  ███ ███     ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ", 
            "  ███████████ ",
            "              ",             
        }; break;
        case '2': lines = {
            "    ██████    ", 
            "  ███    ███  ", 
            " ███      ███ ", 
            "          ███ ", 
            "        ███   ", 
            "      ███     ", 
            "    ███       ", 
            "  ███         ", 
            " ███          ", 
            " ████████████ ",
            "              ",
        }; break;
        case '3': lines = {
            "    ██████    ", 
            "  ███    ███  ", 
            "         ███  ", 
            "         ███  ", 
            "    ██████    ", 
            "         ███  ", 
            "         ███  ", 
            " ███     ███  ", 
            "  ███    ███  ", 
            "    ██████    ",
            "              ",
        }; break;
        case '4': lines = {
            " ███     ███  ", 
            " ███     ███  ", 
            " ███     ███  ", 
            " ███     ███  ", 
            " ███████████  ", 
            "         ███  ", 
            "         ███  ", 
            "         ███  ", 
            "         ███  ", 
            "         ███  ",
            "              ",
        }; break;
        case '5': lines = {
            " ███████████  ", 
            " ███          ", 
            " ███          ", 
            " ███          ", 
            " █████████    ", 
            "         ███  ", 
            "         ███  ", 
            "         ███  ", 
            " ███     ███  ", 
            "  █████████   ",
            "              ",
        }; break;
        case '6': lines = {
            "    ██████    ", 
            "  ███    ███  ", 
            " ███          ", 
            " ███          ", 
            " █████████    ", 
            " ███     ███  ", 
            " ███      ███ ", 
            " ███      ███ ", 
            "  ███    ███  ", 
            "    ██████    ",
            "              ",
        }; break;
        case '7': lines = {
            " ███████████  ", 
            " ███      ██  ", 
            "          ██  ", 
            "          ██  ", 
            "        ███   ", 
            "        ███   ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ", 
            "      ███     ",
            "              ",
        }; break;
        case '8': lines = {
            "   ██████     ", 
            " ███    ███   ", 
            " ██      ███  ", 
            " ██      ███  ", 
            "   ██████     ", 
            " ██      ███  ", 
            " ██      ███  ", 
            " ██      ███  ", 
            " ███    ███   ", 
            "   ██████     ",
            "              ",
        }; break;
        case '9': lines = {
            "    ██████    ", 
            "  ███    ███  ", 
            " ███      ██  ", 
            " ███      ██  ", 
            "  ███    ███  ", 
            "    ████████  ", 
            "          ██  ", 
            "          ██  ", 
            "  ███    ███  ", 
            "    ██████    ",
            "              ",
        }; break;
        case ':': lines = {
            "              ", 
            "              ", 
            "     ████     ", 
            "     ████     ", 
            "              ", 
            "              ", 
            "     ████     ", 
            "     ████     ", 
            "              ", 
            "              ",
            "              ",
        }; break;
        default: lines = {
            "              ",
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              ", 
            "              "
        }; break;
  }

  ftxui::Elements elements;
  for(std::string s : lines){
    elements.push_back(ftxui::text(s));
  }

  return ftxui::vbox(std::move(elements));
}

