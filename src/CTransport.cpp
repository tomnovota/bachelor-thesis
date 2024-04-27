#include "CTransport.h"

bool CTransport::WRITE_TRAJECT = false;

CTransport::CTransport ( double start, double end, double timestep, double r_crit, double r_max, std::deque< std::shared_ptr< CPedestrian > > activeB, std::deque< std::shared_ptr< CPedestrian > > activeL )
: m_Start ( start )
, m_End ( end )
, m_Timestep ( timestep )
, m_R_Crit ( r_crit )
, m_R_Max ( r_max )
, m_ActiveB ( activeB )
, m_ActiveL ( activeL )
, m_Static_Count ( activeB . size() + activeL . size() )
{}

double CTransport::FD ( double mass )
{
    return m_R_Crit >= mass
            ? 1
            : std::max ( 1 - ( mass - m_R_Crit ) / m_R_Max, 0. );
}

bool CTransport::Finished ( void ) const
{
    return m_Waiting . empty()
        && m_ActiveL . size() + m_ActiveB . size() == m_Static_Count;
}

void CTransport::AssignBottleneck ( std::shared_ptr< CBottleneck > bottleneck )
{
    m_Bottleneck_Out = bottleneck;
}

void CTransport::SetWaiting ( const std::vector< CPedestrian > & waiting )
{
    for ( const auto & ped : waiting )
    {
        m_Waiting . push ( std::make_shared< CPedestrian > ( ped ) );
    }
}

void CTransport::Sort ( void )
{
    std::sort ( m_ActiveB . begin(), m_ActiveB . end(), [] ( std::shared_ptr< CPedestrian > l, std::shared_ptr< CPedestrian > r ) { return l -> GetPosition() < r -> GetPosition(); } );

    std::sort ( m_ActiveL . begin(), m_ActiveL . end(), [] ( std::shared_ptr< CPedestrian > l, std::shared_ptr< CPedestrian > r ) { return l -> GetPosition() < r -> GetPosition(); } );
}


CTransportB::CTransportB ( double start, double end, double timestep, double r_crit, double r_max, std::deque< std::shared_ptr< CPedestrian > > activeB, std::deque< std::shared_ptr< CPedestrian > > activeL )
: CTransport ( start, end, timestep, r_crit, r_max, activeB, activeL )
{}

void CTransportB::Arrival ( std::shared_ptr< CPedestrian > ped )
{
    ped -> SetPosition ( m_End );
    m_ActiveL . push_back ( ped );
}

void CTransportB::Departure ( double time )
{
    // new arrivals
    while ( ! m_Waiting . empty()
            && m_Waiting . front() -> GetArrival() <= time )
    {
        m_ActiveB . push_front ( m_Waiting . front() );
        m_Waiting . pop();
    }

    // outgoing
    while ( ! m_ActiveL . empty()
            && m_ActiveL . front() -> GetPosition() <= m_Start )
    {
        m_ActiveL . front() -> WriteEnd ( m_ofstream_end, time );
        m_ActiveL . pop_front();
    }
    
    // moving to next section
    while ( ! m_ActiveB . empty()
            && m_ActiveB . back() -> GetPosition() >= m_End )
    {
        m_ActiveB . back() -> ArrivalBottleneck ( time );
        if ( auto bn = m_Bottleneck_Out . lock() )
            bn -> Arrival ( m_ActiveB . back(), time );
        else
            m_ActiveB . back() -> WriteEnd ( m_ofstream_end, time );

        m_ActiveB . pop_back();
    }
}

std::shared_ptr< CTransport > CTransportB::CreateInstance ( std::string name, std::shared_ptr< CMass > mass ) const
{
    auto instance = std::make_shared< CTransportB > ( * this );

    instance -> m_ofstream_trajectory = std::make_shared< std::ofstream > ( name + ".transportB" );

    instance -> m_ofstream_end = std::make_shared< std::ofstream > ( name + ".transportB_end" );

    if ( ! ( instance -> m_ofstream_trajectory -> is_open()
            && instance -> m_ofstream_trajectory -> is_open() ) )
        throw std::invalid_argument ( "File " + name + ".transportB can't be open." );

    instance -> m_Mass = mass;
    return instance;
}

CTransportL::CTransportL ( double start, double end, double timestep, double r_crit, double r_max )
: CTransport ( start, end, timestep, r_crit, r_max )
{}

void CTransportL::Arrival ( std::shared_ptr< CPedestrian > ped )
{
    ped -> SetPosition ( m_Start );
    m_ActiveB . push_front ( ped );
}

void CTransportL::Departure ( double time )
{
    // new arrivals
    while ( ! m_Waiting . empty()
            && m_Waiting . front() -> GetArrival() <= time )
    {
        m_ActiveL . push_back ( m_Waiting . front() );
        m_Waiting . pop();
    }

    // outgoing
    while ( ! m_ActiveB . empty()
            && m_ActiveB . back() -> GetPosition() >= m_End )
    {
        m_ActiveB . back() -> WriteEnd ( m_ofstream_end, time );
        m_ActiveB . pop_back();
    }
    
    // moving to next section
    while ( ! m_ActiveL . empty()
            && m_ActiveL . front() -> GetPosition() <= m_Start )
    {
        m_ActiveL . front() -> ArrivalBottleneck ( time );
        if ( auto bn = m_Bottleneck_Out . lock() )
                bn -> Arrival ( m_ActiveL . front(), time );
        else
            m_ActiveL . front() -> WriteEnd ( m_ofstream_end, time );

        m_ActiveL . pop_front();
    }
}

std::shared_ptr< CTransport > CTransportL::CreateInstance ( std::string name, std::shared_ptr< CMass > mass ) const
{
    auto instance = std::make_shared< CTransportL > ( * this );

    instance -> m_ofstream_trajectory = std::make_shared< std::ofstream > ( name + ".transportL" );

    instance -> m_ofstream_end = std::make_shared< std::ofstream > ( name + ".transportL_end" );

    if ( ! ( instance -> m_ofstream_trajectory -> is_open()
            && instance -> m_ofstream_trajectory -> is_open() ) )
        throw std::invalid_argument ( "File " + name + ".transportL can't be open." );

    instance -> m_Mass = mass;
    return instance;
}

void CTransport::Step ( double t_n )
{
    std::vector< double > velocities;

    // spočítání aktuální rychlosti chodců B + zápis
    for ( auto ped : m_ActiveB )
    {
        double mass = m_Mass -> GetMassB ( m_ActiveB, ped -> FromDir(), ped -> ToDir() )
                    + m_Mass -> GetMassL ( m_ActiveL, ped -> FromOp(), ped -> ToOp() )
                    - m_Mass -> GetMassSelf ( ped -> GetPosition(), ped -> FromDir(), ped -> ToDir() );

        velocities . push_back ( FD ( mass ) );
    }

    // spočítání aktuální rychlosti chodců L + zápis
    for ( auto ped : m_ActiveL )
    {
        double mass = m_Mass -> GetMassL ( m_ActiveL, ped -> FromDir(), ped -> ToDir() )
                    + m_Mass -> GetMassB ( m_ActiveB, ped -> FromOp(), ped -> ToOp() )
                    - m_Mass -> GetMassSelf ( ped -> GetPosition(), ped -> FromDir(), ped -> ToDir() );

        velocities . push_back ( FD ( mass ) );
    }

    auto vel = velocities . begin();

    // zápis počtu aktivních agentů
    ( * m_ofstream_trajectory ) << m_ActiveB . size() << " ";

    if ( WRITE_TRAJECT )
    {
        for ( auto ped : m_ActiveB )
            ped -> WriteTransport ( m_ofstream_trajectory, * ( vel ++ ) );
    }
    ( * m_ofstream_trajectory ) << std::endl;

    // zápis počtu aktivních agentů
    ( * m_ofstream_trajectory ) << m_ActiveL . size() << " ";

    if ( WRITE_TRAJECT )
    {
        for ( auto ped : m_ActiveL )
            ped -> WriteTransport ( m_ofstream_trajectory, * ( vel ++ ) );
    }
    ( * m_ofstream_trajectory ) << std::endl;

    auto velo = velocities . begin();
    for ( auto ped : m_ActiveB )
        ped -> UpdatePosition ( * ( velo ++ ) * m_Timestep );

    for ( auto ped : m_ActiveL )
        ped -> UpdatePosition ( * ( velo ++ ) * m_Timestep );

    Sort();
}

void CTransport::Premature ( double time )
{
    while ( ! m_ActiveB . empty() )
    {
        m_ActiveB . front() -> WriteEnd ( m_ofstream_end, time );        
        m_ActiveB . pop_front();
    }

    while ( ! m_ActiveL . empty() )
    {
        m_ActiveL . front() -> WriteEnd ( m_ofstream_end, time );
        m_ActiveL . pop_front();
    }
}
