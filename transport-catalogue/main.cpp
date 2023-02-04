#include "json_reader.h"
#include "request_handler.h"
#include "json.h"
#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <tuple>
#include <string_view>
#include <deque>
#include <vector>
#include <sstream>
#include <unordered_map>

using namespace std;

int main() {
	transport::catalog::TransportCatalogue tCatalog;
	transport::render::MapRenderer map;
	transport::request::RequestHandler handler(tCatalog, map);
	transport::json_reader::JsonReader reader(handler, cin);
	reader.HandleDataBase();
	reader.HandleQuery();
	reader.Print(cout);
	return 0;
}