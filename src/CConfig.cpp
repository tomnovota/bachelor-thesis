#include "CConfig.h"

CConfig::CConfig ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep )
: m_Filename ( filename )
, m_Kernels ( kernels )
, m_Timestep ( timestep )
{}

void CConfig::Run ( size_t iter_cnt, double interval_max, CParameters & params )
{
    std::cout << "Param file ["<< m_Filename << "] : Starting " << iter_cnt << " iterations ... " << std::flush;
    for ( size_t i = 0; i < iter_cnt; ++ i )
    {
        std::vector< std::thread > threads;            
        std::string name = m_Filename + std::to_string ( i ) + "/";
        std::filesystem::create_directories ( name );

        m_WaitingB = params . GenerateArrivalsB();
        m_WaitingL = params . GenerateArrivalsL();
        for ( auto ker : m_Kernels )
        {
            threads . push_back ( std::thread ( & CConfig::Simulate, this, ker, interval_max, name + ker -> Name() ) );
        }
        for ( auto & t : threads )
            t . join();
        std::cout << "| " << std::flush;
    }
    std::cout << " ... Finished" << std::endl;
}

CConfigTransportOnly::CConfigTransportOnly ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep, std::shared_ptr< CTransport > t1 )
: CConfig ( filename, kernels, timestep )
, m_Transport1 ( t1 )
{}

void CConfigTransportOnly::Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename )
{
    std::queue< std::shared_ptr< CPedestrian > > waitingL;
    for ( const auto & ped : m_WaitingL )
    {
        waitingL . push ( std::make_shared< CPedestrian > ( ped ) );
    }
    auto t1 = m_Transport1 -> CreateInstance ( filename, kernel );
    t1 -> SetWaiting ( m_WaitingB );

    for ( double t_n = 0;
            ! ( t1 -> Finished() && waitingL . empty() )
            && t_n < interval_max;
        t_n += m_Timestep )
    {
        t1 -> Step ( t_n );
        t1 -> Departure ( t_n );
        while ( ! waitingL . empty() && waitingL . front() -> GetArrival() <= t_n )
        {
            t1 -> Arrival ( waitingL . front() );
            waitingL . pop();
        }
        if ( t_n + m_Timestep >= interval_max )
        {
            std::cout << " (prem) " << std::flush;
            t1 -> Premature ( t_n + m_Timestep );
        }
    }
}

CConfigAll::CConfigAll ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep, std::shared_ptr< CTransport > t1, std::shared_ptr< CTransport > t2, std::shared_ptr< CBottleneck > b1, std::shared_ptr< CBottleneck > b2 )
: CConfig ( filename, kernels, timestep )
, m_Transport1 ( t1 )
, m_Transport2 ( t2 )
, m_BottleneckB ( b1 )
, m_BottleneckL ( b2 )
{}

void CConfigAll::Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename )
{
    auto t1 = m_Transport1 -> CreateInstance ( filename, kernel );
    auto t2 = m_Transport2 -> CreateInstance ( filename, kernel );
    auto b1 = m_BottleneckB -> CreateInstance ( filename );
    auto b2 = m_BottleneckL -> CreateInstance ( filename );

    t1 -> SetWaiting ( m_WaitingB );
    t2 -> SetWaiting ( m_WaitingL );

    t1 -> AssignBottleneck ( b1 );
    t2 -> AssignBottleneck ( b2 );
    b1 -> AssignTransport ( t2 );
    b2 -> AssignTransport ( t1 );

    for ( double t_n = 0;
        ! ( t1 -> Finished()
            && t2 -> Finished()
            && b1 -> Finished()
            && b2 -> Finished() )
            && t_n < interval_max;
            t_n += m_Timestep )
    {
        t1 -> Step ( t_n );
        t2 -> Step ( t_n );
        b1 -> Step ( t_n );
        b2 -> Step ( t_n );
        t1 -> Departure ( t_n );
        t2 -> Departure ( t_n );
    }
}

void CConfigBottleneckOnly::Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename )
{

}
