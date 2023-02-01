#pragma once
#include "transport_catalogue.h"

#include <iostream>

namespace Transport_Catalogue {
	namespace Transport_Catalogue_Output {
		void ProcessingRequests(std::istream&, std::ostream&, TransportCatalogue&);
		namespace detail {
			void PrintStop(std::istream& in, std::ostream& out, TransportCatalogue& tc);
			void PrintRoute(std::istream& in, std::ostream& out, TransportCatalogue& tc);
		}
	}
}