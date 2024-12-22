#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <queue>
#include <regex>
#include <string>
#include <vector>

const uint8_t NUM_LEVELS = 13;
const uint8_t MAX_PLATFORMS_IN_ROUTE = 15;

struct RingLevel {
  uint8_t destinationLevel;
  float levelTime;
};

struct Platform {
  uint8_t platformLevel;
  std::vector<RingLevel> connectedLevels;
};

struct Route {
  std::vector<uint8_t> routePath;
  float routeTime;
  Route();
  Route(const uint8_t &start);
  Route(const Route &route, const RingLevel &next);
};

Route::Route() {
  this->routePath.reserve(MAX_PLATFORMS_IN_ROUTE);
  this->routeTime = 0;
}

Route::Route(const uint8_t &start) {
  this->routePath.reserve(MAX_PLATFORMS_IN_ROUTE);
  this->routePath.push_back(start);
  this->routeTime = 0;
}

Route::Route(const Route &route, const RingLevel &next) {
  this->routePath.reserve(MAX_PLATFORMS_IN_ROUTE);
  for (int i = 0; i < route.routePath.size(); ++i) {
    this->routePath.push_back(route.routePath.at(i));
  }
  this->routePath.push_back(next.destinationLevel);
  this->routeTime = route.routeTime + next.levelTime;
}

using PlatformArr = std::array<Platform, NUM_LEVELS>;

PlatformArr parseTimesFile();
void printPlatforms(const PlatformArr &);
void printRoute(const Route &route);
Route findFastestRoute(const Route &curRoute, const PlatformArr &platforms);
bool allLevelVisitedInRoute(const Route &curRoute);
bool atMaxPathLength(const Route &curRoute);
bool tooManySameLevels(const Route &curRoute);

int main() {
  std::ofstream outFile;
  outFile.open("output.txt");
  outFile.close();

  std::cout << "Parsing input file" << std::endl;
  auto platforms = parseTimesFile();

  for (const auto &platform : platforms) {
    auto baseRoute = Route(platform.platformLevel);
    std::string optimalStr = "Finding optimal path for platform " +
                             std::to_string(platform.platformLevel);

    std::cout << optimalStr << std::endl;
    outFile.open("output.txt", std::ios_base::app);
    outFile << optimalStr << std::endl;
    outFile.close();

    printRoute(findFastestRoute(baseRoute, platforms));
  }
}

PlatformArr parseTimesFile() {
  auto platforms = PlatformArr();

  std::ifstream timeFile("times.txt");
  std::string line;
  std::regex reg("(\\d+) -> (\\d+): (\\d+\\.\\d+)");

  while (std::getline(timeFile, line)) {
    std::smatch match;
    if (std::regex_search(line, match, reg)) {
      auto curPlatform = std::stoi(match[1]);
      auto nextPlatform = std::stoi(match[2]);
      auto levelTime = std::stof(match[3]);

      platforms[curPlatform - 1].platformLevel = curPlatform;
      platforms[curPlatform - 1].connectedLevels.push_back(
          {uint8_t(nextPlatform), levelTime});
    } else {
      std::cout << "Line in times.txt is not valid: " << line << std::endl;
    }
  }

  return platforms;
}

void printPlatforms(const PlatformArr &platforms) {
  for (const auto &platform : platforms) {
    std::cout << "Platform number: " << std::to_string(platform.platformLevel)
              << std::endl;
    for (const auto &level : platform.connectedLevels) {
      std::cout << "  Connected level: "
                << std::to_string(level.destinationLevel)
                << " with time: " << level.levelTime << std::endl;
    }
  }
}

void printRoute(const Route &route) {
  std::ofstream outFile;
  outFile.open("output.txt", std::ios_base::app);

  if (route.routePath.size() == 0) {
    std::string impossibleStr = "No " +
                                std::to_string(MAX_PLATFORMS_IN_ROUTE - 1) +
                                " level routes possible from starting platform";
    outFile << impossibleStr << std::endl;
    std::cout << impossibleStr << std::endl;
    return;
  }

  std::string routeTimeStr = "Route time: " + std::to_string(route.routeTime);
  std::string routePathStr = "Route path: ";

  for (int i = 0; i < route.routePath.size() - 1; ++i) {
    routePathStr += std::to_string(route.routePath[i]) + " -> ";
  }
  routePathStr += std::to_string(route.routePath[route.routePath.size() - 1]);

  outFile << routeTimeStr << std::endl;
  outFile << routePathStr << std::endl;

  std::cout << routeTimeStr << std::endl;
  std::cout << routePathStr << std::endl;
}

Route findFastestRoute(const Route &startRoute, const PlatformArr &platforms) {
  auto routeComp = [](Route left, Route right) {
    return left.routeTime > right.routeTime;
  };
  std::priority_queue<Route, std::vector<Route>, decltype(routeComp)> queue(
      routeComp);

  queue.push(startRoute);

  while (!queue.empty()) {
    auto curRoute = queue.top();
    queue.pop();

    if (allLevelVisitedInRoute(curRoute))
      return curRoute;

    if (atMaxPathLength(curRoute))
      continue;

    auto &routePlatNum = curRoute.routePath[curRoute.routePath.size() - 1];
    auto &routePlat = platforms[routePlatNum - 1];

    for (const auto &connectedPlat : routePlat.connectedLevels) {
      auto nextRoute = Route(curRoute, connectedPlat);
      if (!tooManySameLevels(nextRoute))
        queue.push(nextRoute);
    }
  }

  return Route();
}

bool allLevelVisitedInRoute(const Route &curRoute) {
  auto platformCheck = std::array<bool, NUM_LEVELS>();
  for (const auto &platform : curRoute.routePath) {
    platformCheck[platform - 1] = true;
  }

  for (const auto &visited : platformCheck) {
    if (!visited)
      return false;
  }

  return true;
}

bool atMaxPathLength(const Route &curRoute) {
  return curRoute.routePath.size() >= MAX_PLATFORMS_IN_ROUTE;
}

bool tooManySameLevels(const Route &curRoute) {
  auto platformCheck = std::array<uint8_t, NUM_LEVELS>();
  for (const auto &platform : curRoute.routePath) {
    platformCheck[platform - 1]++;
  }

  int doubleVisits = 0;
  for (const auto &visited : platformCheck) {
    if (visited >= 3)
      return true;
    else if (visited == 2)
      ++doubleVisits;
  }

  return doubleVisits > 2;
}
