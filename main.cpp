#include "Server.h"
#include "Stream.h"
#include <map>

struct Location
{
  string path;
  map<string, string> directives;
};

class Server2
{
  public:
    string port;
    string host;
    SockAddrIn address;
    int sock;
    vector<Location> locations;
};

bool skipEmptyLines(string& line) {
  string::size_type start = line.find_first_not_of(" \t\n\r\f\v");
  string::size_type end = line.find_last_not_of(" \t\n\r\f\v");
  if (start == string::npos || end == string::npos) {
    return true;
  }
  line = line.substr(start, end - start + 1);
  if (line.empty() || line[0] == '#') {
    return true;
  }
  if(line.find("server {") != string::npos)
    return true;
  return false;
}

Server2 parser(char *file, Server2& server) {
  (void)server;
  ifstream in(file);
  if (!in.is_open() || in.bad() || in.fail()) {
    cerr << "Error: Could not open file, using default configuration" << endl;
    // server = Server(); // construtor void configura com valores default
    throw(runtime_error("Could not open file"));
  }
  string line;
  Location currentLocation;
  Server2 config;
  bool inLocation = false;
  while(getline(in, line))
  {
    if(skipEmptyLines(line))
      continue;
    else if(line.find("}") != string::npos)
    {
      // if(!inLocation)
      // {
      //   cerr << "Error: file is not well formatted, using default configurations." << endl;
      //   // server = Server();
      //   return ;
      // }
      if(inLocation)
      {
        config.locations.push_back(currentLocation);
        currentLocation = Location();
        inLocation = false;
      }
    }
    else if(line.find("listen") != string::npos)
    {
      string value = line.substr(line.find(" ") + 1);
      try
      {
        config.port = value;
      }
      catch(const exception& e)
      {
        cerr << "Error: Invalid listen value: " << value << endl;
        // server = Server();
        throw(runtime_error("Invalid listen value"));
      }
    }
    else if(line.find("index") != string::npos)
    {
      string value = line.substr(line.find(" ") + 1);
      if(inLocation)
        currentLocation.directives["index"] = value;
      else
        config.host = value;
    }
    else if(line.find("root") != string::npos)
    {
      string value = line.substr(line.find(" ") + 1);
      if(inLocation)
        currentLocation.directives["root"] = value;
      else
        config.host = value;
    }
    else if(line.find("autoindex") != string::npos)
    {
      string value = line.substr(line.find(" ") + 1);
      if(inLocation)
        currentLocation.directives["autoindex"] = value;
    }
    else if(line.find("location") != string::npos)
    {
      inLocation = true;
      currentLocation.path = line.substr(line.find(" ") + 1);
      if(!currentLocation.path.empty() && currentLocation.path[currentLocation.path.size() - 1] == '{')
        currentLocation.path.erase(currentLocation.path.size() - 1);
    }
    else
    {
      if(inLocation)
      {
        string::size_type spacePos = line.find(" ");
        string directive = line.substr(0, spacePos);
        string value = line.substr(spacePos + 1);
        currentLocation.directives[directive] = value;
      }
    }
  }
  in.close();
  return config;
}

/* void server(int c, char **v) {
  if (c == 1) {
  	Server server;
	  server.run();
  }
  else {
    Server server;
    parser(v[1], server);
    server.run();
  }
} */


int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <config-file>" << endl;
        return 1;
    }

    try {
        Server2 server = parser(argv[1], server);

        cout << "Port: " << server.port << endl;
        cout << "Host: " << server.host << endl;

        for (vector<Location>::const_iterator it = server.locations.begin(); it != server.locations.end(); ++it) {
            const Location& loc = *it;
            cout << "Location: " << loc.path << endl;
            for (map<string, string>::const_iterator dit = loc.directives.begin(); dit != loc.directives.end(); ++dit) {
                cout << "  " << dit->first << ": " << dit->second << endl;
            }
        }
    } catch (const exception& e) {
        cerr << "Error parsing config file: " << e.what() << endl;
        return 1;
    }

    return 0;
}