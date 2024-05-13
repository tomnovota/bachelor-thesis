#include <string>

#include "CParameters.h"
#include "CTransport.h"
#include "CConfig.h"

int main ( int argc, char ** argv )
{
    size_t iter_count = 1;
    std::string filename = "params/full_simulation";
    CTransport::WRITE_TRAJECT = false;
    if ( argc > 2 )
    {
        iter_count = std::stoi ( argv [ 1 ] );
        filename = argv [ 2 ];
    }
    if ( argc > 3 && std::stoi ( argv [ 3 ] ) == 1 )
    {
        CTransport::WRITE_TRAJECT = true;
    }
    CParameters parser;
    try
    {
        auto config = parser . Read ( filename );
        config -> Run ( iter_count, 2000, parser );
    }
    catch ( const std::exception & e )
    {
        std::cerr << e.what() << '\n';
    }
    return 0;
}


