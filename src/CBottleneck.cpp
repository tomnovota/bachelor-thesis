#include "CBottleneck.h"

CBottleneck::CBottleneck ( double distance, double body_width )
{
    m_Path . resize ( std::ceil ( std::abs ( distance ) / body_width ), nullptr );
}

bool CBottleneck::Finished ( void ) const
{
    return m_Queue . empty() && m_Update_Order . empty();
}

void CBottleneck::Arrival ( std::shared_ptr< CPedestrian > ped, double time )
{
    m_Queue . push ( ped );
    if ( m_Queue . size() == 1 )
    {
        AssignUpdateTime ( time, m_Path . size() );
    }
}

void CBottleneck::Departure ( double time )
{
    if ( auto tp = m_Transport_Out . lock() )
    {
        m_Path . back() -> ArrivalTransport ( time );
        tp -> Arrival ( m_Path . back() );
    }
    else
        throw std::invalid_argument ( "Transport pointer destroyed!" );
}

void CBottleneck::AssignTransport ( std::shared_ptr< CTransport > transport )
{
    m_Transport_Out = transport;
}

void CBottleneck::AssignUpdateTime ( double time, std::size_t position )
{
    if ( position == m_Path . size() )
    {
        m_Queue . front() -> NewUpdate ( time );
        PlaceInOrder ( m_Queue . front() -> NextUpdate(), m_Path . size() );
    }
    else
    {
        m_Path [ position ] -> NewUpdate ( time );
        PlaceInOrder ( m_Path [ position ] -> NextUpdate(), position );
    }
}

CBottleneckB::CBottleneckB ( double distance, double body_width )
: CBottleneck ( distance, body_width )
{}

std::shared_ptr< CBottleneck > CBottleneckB::CreateInstance ( std::string filename ) const
{
    auto instance = std::make_shared< CBottleneckB > ( * this );
    instance -> m_ofstream = std::make_shared< std::ofstream > ( filename + ".bottleneckB" );
    if ( ! instance -> m_ofstream -> is_open() )
        throw std::invalid_argument ( "File " + filename + ".bottleneckB can't be open." );

    return instance;
}

void CBottleneckB::Log ( double time )
{
    ( * m_ofstream )
        << time << ":"
        << m_Queue . size() << ":";

    std::size_t pos = 0;
    for ( auto ped : m_Path )
    {
        if ( ped != nullptr )        
        {
            ped -> WriteBottleneck ( m_ofstream, pos );
        }
        ++ pos;
    }   
    ( * m_ofstream )
        << std::endl;
}

CBottleneckL::CBottleneckL ( double distance, double body_width )
: CBottleneck ( distance, body_width )
{}

std::shared_ptr< CBottleneck > CBottleneckL::CreateInstance ( std::string filename ) const
{
    auto instance = std::make_shared< CBottleneckL > ( * this );

    instance -> m_ofstream = std::make_shared< std::ofstream > ( filename + ".bottleneckL" );
    if ( ! instance -> m_ofstream -> is_open() )
        throw std::invalid_argument ( "File " + filename + ".bottleneckL can't be open." );

    return instance;
}

void CBottleneckL::Log ( double time )
{
    ( * m_ofstream ) << std::fixed << std::setprecision ( 2 )
        << time << ":"
        << m_Queue . size() << ":";

    std::size_t pos = 0;
    for ( auto ped = m_Path . rbegin(); ped != m_Path . rend(); ++ ped )
    {
        if ( ( * ped ) != nullptr )        
        {
            ( * ped ) -> WriteBottleneck ( m_ofstream, pos );
        }
        ++ pos;
    }   
    ( * m_ofstream )
        << std::endl;
}

void CBottleneck::Step ( double t_n )
{
    while ( ! m_Update_Order . empty() )
    {
        double time_event;
        size_t position_event = m_Update_Order . front();

        // Čas updatu
        time_event = ( position_event == m_Path . size() )
                    ? ( m_Queue . front() -> NextUpdate() )
                    : ( m_Path [ position_event ] -> NextUpdate() );

        if ( time_event > t_n )
            break;

        m_Update_Order . pop_front();
        Update ( time_event, position_event );
        Log ( time_event );
    }
}

void CBottleneck::Update ( double time, std::size_t position )
{
    if ( position == m_Path . size() ) // queue
    {
        if ( m_Path [ 0 ] == nullptr ) // free place
        {
            m_Path [ 0 ] = m_Queue . front(); // move
            m_Queue . pop();
            AssignUpdateTime ( time, 0 ); // next update time
            if ( ! m_Queue . empty() ) // assign time for next waiting ped
                AssignUpdateTime ( time, m_Path . size() );
        }
        else
            AssignUpdateTime ( time, m_Path . size() ); // next try
    }
    else if ( position < m_Path . size() - 1 ) // mid
    {
        if ( m_Path [ position + 1 ] == nullptr ) // free place
        {
            m_Path [ position + 1 ] = m_Path [ position ]; // move
            m_Path [ position ] = nullptr; // clear
            AssignUpdateTime ( time, position + 1 ); // next update time
        }
        else
            AssignUpdateTime ( time, position ); // next try
    }
    else if ( position == m_Path . size() - 1 ) // end
    {
        Departure ( time );
        m_Path . back() = nullptr; // clear
    }
}

void CBottleneck::PlaceInOrder ( double time, std::size_t position )
{
    auto it = std::lower_bound ( m_Update_Order . begin(), m_Update_Order . end(), time,
    [ this ] ( std::size_t l, double t )
    {
        double l_update = ( l < this -> m_Path . size() )
            ? this -> m_Path [ l ] -> NextUpdate()
            : this -> m_Queue . front() -> NextUpdate();

        return l_update < t;
    } );
    m_Update_Order . insert ( it, position );
}


