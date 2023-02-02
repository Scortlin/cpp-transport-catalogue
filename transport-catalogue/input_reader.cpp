#include "input_reader.h"

#include <string_view>
#include <tuple>
#include <iostream>

using namespace std;

namespace Transport_Catalogue {
    namespace Transport_Catalogue_Input {
        void FillCatalog(istream& is, TransportCatalogue& tc) {
            int vvod_count;
            vector<string> bus_query_s;
            vector<Transport_Catalogue::detail::StopFindResult> busstop_coord_stopdists;

            is >> vvod_count;
            for (int i = 0; i < vvod_count; ++i) {
                string key, bus_stop;
                is >> key;
                getline(is, bus_stop);
                if (key == "Stop"s) {
                    busstop_coord_stopdists.push_back(detail::FillStop(bus_stop.substr(1)));
                    tc.AddStop(busstop_coord_stopdists.back().name, busstop_coord_stopdists.back().coord);
                }
                else if (key == "Bus"s) {
                    bus_query_s.push_back(bus_stop.substr(1));
                }
            }
            for (auto& bus_str : bus_query_s) {
                auto [name, stops, circle_key] = detail::FillRoute(bus_str);
                tc.AddRoute(name, stops, circle_key);
            }

            for (auto& busstop_coord_stopdist : busstop_coord_stopdists) {
                for (auto& stopdist : busstop_coord_stopdist.otherstop_dist) {
                    auto temp = tc.FindStop(stopdist.first);
                    tc.SetDistanceBetweenStops(busstop_coord_stopdist.name, temp->name, stopdist.second);
                }
            }
        }
        namespace detail {
            Transport_Catalogue::detail::StopFindResult FillStop(string_view query) {
                Transport_Catalogue::detail::StopFindResult result;
                size_t iter = query.find(':');
                result.name = static_cast<string>(query.substr(0, iter));
                query.remove_prefix(iter + 2);
                iter = query.find(',');
                string str;
                result.coord.lat = stod(static_cast<string>(query.substr(0, iter)));
                query.remove_prefix(iter + 2);

                if (iter = query.find(','); iter != query.npos) {
                    result.coord.lng = stod(static_cast<string>(query.substr(0, iter)));
                    query.remove_prefix(iter + 2);

                    iter = query.find(',');

                    while (iter != query.npos) {
                        iter = query.find('m');
                        uint32_t dist = stoi(static_cast<string>(query.substr(0, iter)));
                        query.remove_prefix(iter + 5);
                        iter = query.find(',');
                        result.otherstop_dist[static_cast<string>(query.substr(0, iter))] = dist;
                        query.remove_prefix(iter + 2);
                        iter = query.find(',');
                    }

                    iter = query.find('m');
                    uint32_t dist = stoi(static_cast<string>(query.substr(0, iter)));
                    query.remove_prefix(iter + 5);
                    result.otherstop_dist[static_cast<string>(query)] = dist;
                }
                else {
                    result.coord.lng = stod(static_cast<string>(query));
                }
                return result;
            }
            tuple<string, vector<string>, bool > FillRoute(string_view query) {
                vector<string> unique_stops;
                bool circle_key = true;
                size_t iter = query.find(':');
                string bus = static_cast<string>(query.substr(0, iter));
                query.remove_prefix(iter + 1); \
                    iter = query.find('-');
                if (iter != query.npos) {
                    circle_key = false;
                    unique_stops = FillUniqueStops(query, circle_key);
                }
                else {
                    unique_stops = FillUniqueStops(query, circle_key);
                    unique_stops.pop_back();
                }
                return { bus,unique_stops, circle_key };
            }

            vector<string> FillUniqueStops(string_view query, bool circle_flag) {
                vector<string> unique_stops;
                size_t iter = (circle_flag ? query.find('>') : query.find('-'));
                while (iter != query.npos) {
                    string_view station = query.substr(1, iter);
                    station.remove_suffix(2);
                    query.remove_prefix(iter + 1);
                    iter = circle_flag ? query.find('>') : query.find('-');
                    unique_stops.push_back(static_cast<string>(station));
                }
                query.remove_prefix(1);
                unique_stops.push_back(static_cast<string>(query));
                return unique_stops;
            }

        }
    }
}