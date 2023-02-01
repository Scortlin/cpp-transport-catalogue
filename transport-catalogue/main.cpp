#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
using namespace std;
using namespace Transport_Catalogue;

int main()
{
    TransportCatalogue ob1;
    Transport_Catalogue_Input::FillCatalog(cin, ob1);
    Transport_Catalogue_Output::ProcessingRequests(cin, cout, ob1);
    return 0;
}