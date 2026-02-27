#include "storage.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>
#include <vector>
using json = nlohmann::json;

void save(std::vector<std::pair<int, std::string>>& time_habits){
  json data = time_habits;

  std::ofstream file("data.json");
  file << data.dump(2);
}

void load(std::vector<std::pair<int, std::string>>& time_habits){
  std::ifstream file("data.json");
  if(!file.is_open()) return;

  json data;
  file >> data;
  time_habits = data.get<std::vector<std::pair<int, std::string>>>();
}
