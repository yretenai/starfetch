#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>
#include <string>
#include <ctime>
#include <filesystem>
#include <random>
#include <regex>
#include <list>
//#include <unistd.h> for getpid()
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

static void setColor(const string &color); //sets given color to the REQUESTED_COLOR variable to colorize the output constellation
static inline void PrintConst(const string &pathc);  //formats the template file with the requested data and prints out the constellation info
static string RandomConst(const vector<string> &directories);   //select a random constellation from the available ones
static vector<string> LoadConfig(const string &config_path); //loads the config file
static void TestConst(const vector<string> &directories); //prints every constellation
static void PrintList(const vector<string> &directories);   //prints out the list of the available constellations
static void Error(const char *err, int type);   //shows an error message
static void Help();    //prints out the help message

static string REQUESTED_COLOR = "\033[1;37m"; // white color

static string SEP(1, filesystem::path::preferred_separator);
#define STR1(x)  #x
#define STR(x) STR1(x)
static string path = STR(STARFETCH_PREFIX) + SEP + "share"s + SEP + "starfetch"s + SEP;

int main(int argc, char *argv[])
{
#ifdef _WIN32
	string config_path = "./res/starfetch.json";
#else
	string config_path;
	char* xdg_config_dir = getenv("XDG_CONFIG_DIR");
	if(xdg_config_dir == nullptr) {
		config_path = string(getenv("HOME")) + SEP + ".config";
	} else {
		config_path = string(xdg_config_dir);
	}

	config_path += SEP + "starfetch.json";
#endif
	string pathc = path;
  vector<string> directories = LoadConfig(config_path);
  if (directories.empty()) {
    directories.push_back("constellations"s);
    directories.push_back("norse-constellations"s);
  }

  if(argc == 1)   //if there's no additional arguments
    pathc += RandomConst(directories); //selects a random constellation
  else
    switch(argv[1][1])  //gets the time of the argument (the 'n' in "-n")
    {
      case 'p':
        {
          if(argc < 3) Error(" ", 0); //if the user requested a '-n' argument but didn't provide a name, an error occurs
          pathc += directories[0] + SEP + argv[2] + ".json"; //updating the path to the constellations folder and adding the name of the requested constallation to the pathc
        }
        break;

        // Refactored -n
      case 'n':
        {
          if(argc < 3) Error(" ", 0); //if the user requested a '-p' argument but didn't provide a name, an error occurs
          if(argc < 4) {
            pathc += directories[0] + SEP + string(argv[2]) + ".json"; //updating the path to the constellations folder and adding the name of the requested constallation to the pathc
          } else {
            string tradition = static_cast<string>(argv[2]);
            auto elem = std::find(directories.begin(), directories.end(), tradition);
            if (elem == directories.end()) {
              Error(" ", 0);
            } else {
              pathc += *elem + SEP + string(argv[3]) + ".json"; //updating the path to the constellations folder and adding the name of the requested constallation to the pathc
            }
          }
        }
        break;

      case 'h':
        Help();
        return EXIT_SUCCESS;
      case 't': // test, print every constellation
        TestConst(directories);
        return EXIT_SUCCESS;
      case 'r':
        pathc += RandomConst(directories); //with the '-r' option, it selects a random constellation
        break;
      case 'l':
        PrintList(directories);
        return EXIT_SUCCESS;
      case 'c':
        {
          if (argc == 2)
          {
            cout << "Available colors are: black, white, cyan, magenta, yellow, red, blue" << endl;
            return EXIT_SUCCESS;
          }
          else if (argc == 3 || argc == 4)
          {
            setColor(static_cast<string>(argv[2]));

            if (argc == 4 && !strcmp(argv[3], "-l"))
            {
              PrintList(directories);
              return EXIT_SUCCESS;
            }

            pathc += RandomConst(directories);
          }
          else
          {
            pathc += directories[0] + SEP + argv[4] + ".json"; //updating the path to the constellations folder and adding the name of the requested constellation to the pathc
            setColor(static_cast<string>(argv[2]));
          }
        }
        break;
      default:
        Error(argv[1], 1);  //if the reqeusted option isn't recognized, an error occours
        break;
    }

  PrintConst(pathc);  //prints the constellation
  return EXIT_SUCCESS;
}

static void setColor(const string &color)
{
  if (color == "black")
    REQUESTED_COLOR = "\033[1;30m";
  else if (color == "white")
    REQUESTED_COLOR = "\033[1;37m";
  else if (color == "cyan")
    REQUESTED_COLOR = "\033[1;36m";
  else if (color == "magenta")
    REQUESTED_COLOR = "\033[1;35m";
  else if (color == "yellow")
    REQUESTED_COLOR = "\033[1;33m";
  else if (color == "red")
    REQUESTED_COLOR = "\033[1;31m";
  else if (color == "blue")
    REQUESTED_COLOR = "\033[1;34m";
}

static inline void PrintConst(const string &pathc)
{
  ifstream c(pathc);  //opens the file containing constellation info
  ifstream f(path+"template");    //opens the output template file
  stringstream strStream;
  string s, l;
  json j;

  if(f.is_open())
  {
    strStream << f.rdbuf(); //read the template
    s = strStream.str();    //converts it to a string
    replace(s.begin(), s.end(), '^', '\033');   //replace '^' with the '\e' to print bold/colored text
    f.close();  //closes the template
  }

  if(c.is_open())
  {
    c >> j;     //parse the selected JSON file
                //fills the template with dats
    s.replace(s.find("%0"), string("%0").size(), j["title"].get<string>());
    s.replace(s.find("%11"), string("%11").size(), j["name"].get<string>());
    s.replace(s.find("%12"), string("%12").size(), j["quadrant"].get<string>());
    s.replace(s.find("%13"), string("%13").size(), j["right ascension"].get<string>());
    s.replace(s.find("%14"), string("%14").size(), j["declination"].get<string>());
    s.replace(s.find("%15"), string("%15").size(), j["area"].get<string>());
    s.replace(s.find("%16"), string("%16").size(), j["main stars"].get<string>());

    //renders the constellation's graph from the coordinates specified in the JSON file
    for(int i=1;i<=10;i++)  //for each of the lines (10)
    {
      l="";
      for(int k=1;k<=22;k++)  //for each of the columns of the graph (22)
                              //if the JSON file specifies a star at position k
        if(j["graph"]["line"+to_string(i)].find(to_string(k)) != j["graph"]["line"+to_string(i)].end())
          l+=REQUESTED_COLOR + j["graph"]["line"+to_string(i)][to_string(k)].get<string>() + "\033[0;0m"; //put the star (which is stored into the JSON fine, might change this in the future)
        else
          l+=" "; //put a space

      //insert the line into the template
      s.replace(s.find("%"+to_string(i)), string("%"+to_string(i)).size(), l);
      s = std::regex_replace(s, std::regex("requestedColor"), REQUESTED_COLOR);
    }

    c.close();
    cout << s << endl;  //prints the output
  }else
    Error("", 2);
}

static vector<string> LoadConfig(const string &config_path) {
  if(!filesystem::exists(config_path)) {
    return {"constellations", "norse-constellations"};
  }

  ifstream f(config_path);
  json j; // {"groups": ["constellations", "norse-constellations"], "color": "white"}
  vector<string> directories;

  if(f.is_open())
  {
    f >> j;
    for (const auto &group : j["groups"]) {
      directories.push_back(group.get<string>());
    }
    auto color = j["color"].get<string>();
    setColor(color);
    f.close();
  }

  return directories;
}

// the previous version was biased, because it was selecting a random directory.
// this made it so that the "norse constellations" group, which had one constellation.
// this means that the single norse constellation had a 50% chance to be selected.
// while the "constellations" group which had 39 constellations only had a 2.56% chance for each constellation.
// this version selects a random constellation from the available ones, so each constellation has the same probability to be selected (1/40 = 2.5%)
static string RandomConst(const vector<string> &directories) {
  string s;
  unsigned long int seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::mt19937 mt(seed);
  std::vector<std::string> constellations;

  for (const auto &dir : directories) {
    for (const auto & entry : filesystem::directory_iterator(path + dir + SEP)) {
      s = entry.path().filename().string();
      if (!s.ends_with(".json")) {
        continue;
      }

      constellations.push_back(dir + SEP + s);
    }
  }

  return constellations[mt() % constellations.size()];
}

static void TestConst(const vector<string> &directories) {
  string s;

  for (const auto &dir : directories) {
    for (const auto & entry : filesystem::directory_iterator(path + dir + SEP)) {
      s = entry.path().filename().string();
      if (!s.ends_with(".json")) {
        continue;
      }

      PrintConst(path + dir + SEP + s);
      cout << endl;
    }
  }
}


static void PrintList(const vector<string> &directories) {
  string s;

  //prints every constellation name from the files name in the directories array
  for (const auto &dir : directories){
    cout << "\n" + REQUESTED_COLOR + "✦ available " + dir + "\033[0;0m:" << endl;
    for (const auto & entry : filesystem::directory_iterator(path + dir + SEP))
    {
      s = entry.path().filename().string(); //from "/usr/local/opt/starfetch/res/constellations/xxxxxx" to "xxxxxx"
      if (!s.ends_with(".json")) {
        continue;
      }
      s = s.substr(0, s.length()-5);
      
      cout << REQUESTED_COLOR + "  - " + s + "\033[0;0m" << endl;
    }
  }
}

static void Error(const char *err, int code)
{
  switch(code)    //each error has a specific code
  {
    case 0: //0 for the missing input
      cout << "Error: you must input a constellation name after -n." << endl << endl;
      break;
    case 1: //1 for the invalid argument
      cout << "Error: '" << err << "' isn't a valid argument." << endl << endl;
      break;
    case 2: //2 for the invalid constellation name
      cout << "Error: the constellation you asked for isn't recognized." << endl << endl;
      break;
  }

  Help(); //after any error occours, the help message is shown
}

static void Help()
{
  ifstream f(path + "help_message.txt");
  cout << f.rdbuf();
  f.close();
  exit(EXIT_SUCCESS);
}
