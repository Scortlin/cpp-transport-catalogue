#include "stat_reader.h"

#include <iomanip>

using namespace std;
namespace Transport_Catalogue {
	namespace Transport_Catalogue_Output {
		void ProcessingRequests(istream& in, ostream& out, TransportCatalogue& tc) {
			int requests_number;
			in >> requests_number;
			for (int i = 0; i < requests_number; ++i) {
				string key;
				in >> key;
				if (key == "Bus"s) {
					detail::PrintRoute(in, out, tc);
				}
				else if (key == "Stop"s) {
					detail::PrintStop(in, out, tc);
				}
			}
		}
		namespace detail {
			void PrintStop(istream& in, ostream& out, TransportCatalogue& tc) {
				string bus_stop;
				getline(in, bus_stop);
				bus_stop = bus_stop.substr(1);
				if (!tc.FindStop(bus_stop)) {
					out << "Stop "s << bus_stop << ": not found"s << '\n';
				}
				else {
					auto busstops = tc.StopInfo(bus_stop);
					if (!busstops.size()) {
						out << "Stop "s << bus_stop << ": no buses"s << '\n';
					}
					else {
						out << "Stop "s << bus_stop << ": buses"s;
						for (auto stop : tc.StopInfo(bus_stop)) {
							out << ' ' << stop;
						}
						out << '\n';
					}
				}
			}
			void PrintRoute(istream& in, ostream& out, TransportCatalogue& tc) {
				string bus;
				getline(in, bus);
				bus = bus.substr(1);
				if (!tc.FindRoute(bus)) {
					out << "Bus "s << bus << ": not found"s << '\n';
				}
				else {
					RoutInfoS rout_info = tc.RouteInfo(bus);
					out << setprecision(6);
					out << "Bus "s << bus << ": "s << rout_info.rout_stops_count << " stops on route, "s << rout_info.unic_rout_stop_count << " unique stops, "s
						<< rout_info.route_length << " route length, "s << rout_info.curvature << " curvature\n"s;
				}
			}
		}
	}
}