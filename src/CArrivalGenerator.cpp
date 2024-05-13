#include "CArrivalGenerator.h"

double CArrivalGenerator::CalcValue ( double time, const std::vector< double > & coeffs )
{
    double res = 0;
    // int deg = 0;
    for ( const auto & coef : coeffs )
    {
        res = res * time + coef;
    }
    return res;
}

std::vector< double > CArrivalGenerator::GenerateEvents ( double interval, double lambda, std::vector< double > coeffs )
{
    // random number generator
    std::random_device rd;
    std::mt19937 gen ( rd() );
    std::exponential_distribution<> exponential_d ( lambda );

    std::vector< double > events_homo;
    double time, prob;
    while ( true )
    {
        time = exponential_d ( gen ) + ( events_homo . empty() ? 0 : events_homo . back() );
        if ( time > interval )
            break;

        events_homo . push_back ( time );
    }
    if ( ! coeffs . empty() )
    {
        std::vector< double > events;
        for ( auto time : events_homo )
        {
            prob = CalcValue ( time, coeffs ) / lambda;
            std::bernoulli_distribution bern_d ( prob );
            if ( bern_d ( gen ) ) 
            {
                events . push_back ( time );
            }
        }
        return events;
    }
    return events_homo;
}