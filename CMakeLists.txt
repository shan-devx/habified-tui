find_package(ftxui REQUIRED)
find_package(nlohmann_json REQUIRED)
add_executable(habified-tui src/main.cpp src/storage.cpp)
target_link_libraries(habified-tui
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE nlohmann_json::nlohmann_json
)
