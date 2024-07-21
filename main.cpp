#include "Server.h"
#include "Stream.h"

void parser(Server & server) {
  Stream  data("Makefile");
  cout << reinterpret_cast<char *>(data.getStream());
  (void)server;
  cout << "parser" << endl;
}

void server(int c, char **v) {
  if (c == 1) {
  	Server server;
	  server.run();
  }
  else {
    Server server(v[1]);
    parser(server);
    server.run();
  }

}
int main(int c, char **v) {
  server(c, v);
	return 0;
}

