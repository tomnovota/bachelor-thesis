#include "CPedestrian.h"

constexpr bool PRINT = false;

CPedestrian::CPedestrian ( size_t id, double departure, double position, double d1_dir, double d2_dir, double d1_op, double d2_op, double speed, double body )
: m_ID ( id )
, m_Speed_opt ( speed )
, m_Arrival_Start ( departure )
, m_Position_Curr ( position )
, m_d1_dir ( d1_dir )
, m_d2_dir ( d2_dir )
, m_d1_op ( d1_op )
, m_d2_op ( d2_op )
, m_Exp_Distribution ( std::abs ( speed ) / body )
{}

double CPedestrian::GetArrival ( void ) const { return m_Arrival_Start; }

double CPedestrian::GetPosition ( void ) const { return m_Position_Curr; }

std::size_t CPedestrian::GetID ( void ) const { return m_ID; }

void CPedestrian::SetPosition ( double pos ) { m_Position_Curr = pos; }

void CPedestrian::WriteTransport ( std::shared_ptr< std::ofstream > ofs, double coeff )
{
    ( * ofs )
        << m_ID << ","
        << std::fixed << std::setprecision ( 2 ) << std::abs ( m_Speed_opt * coeff ) << ","
        << std::fixed << std::setprecision ( 2 ) << m_Position_Curr << " ";
}

void CPedestrian::WriteEnd ( std::shared_ptr< std::ofstream > ofs, double time )
{
    if ( PRINT )
        std::cout << "[" << ( m_Speed_opt > 0 ? "B" : "L" ) << "] "
        << m_ID << " (" << std::fixed << std::setprecision ( 2 ) << std::abs ( m_Speed_opt ) << " m/s)"
        << " Departure "
        << time << " s"
        << std::endl;
    ( * ofs )
        << std::fixed << std::setprecision ( 2 ) << m_Speed_opt << ","
        << std::fixed << std::setprecision ( 2 ) << time << ","
        << std::fixed << std::setprecision ( 2 ) << m_Arrival_Start << " ";
}

void CPedestrian::WriteBottleneck ( std::shared_ptr< std::ofstream > ofs, std::size_t pos )
{
    ( * ofs )
        << std::fixed << std::setprecision ( 2 )
        << pos << ","
        << m_ID << " ";
}

double CPedestrian::FromDir ( void ) const
{
    return m_Speed_opt > 0
        ? m_Position_Curr + m_d1_dir
        : m_Position_Curr - m_d2_dir;
}

double CPedestrian::ToDir ( void ) const
{
    return m_Speed_opt > 0
        ? m_Position_Curr + m_d2_dir
        : m_Position_Curr - m_d1_dir;
}

double CPedestrian::FromOp ( void ) const
{
    return m_Speed_opt > 0
        ? m_Position_Curr + m_d1_op
        : m_Position_Curr - m_d2_op;
}

double CPedestrian::ToOp ( void ) const
{
    return m_Speed_opt > 0
        ? m_Position_Curr + m_d2_op
        : m_Position_Curr - m_d1_op;
}

void CPedestrian::UpdatePosition ( double coef )
{
    m_Position_Curr += m_Speed_opt * coef;
}

void CPedestrian::ArrivalBottleneck ( double t )
{
    if ( PRINT )
        std::cout << "[" << ( m_Speed_opt > 0 ? "B" : "L" ) << "] "
        << m_ID << " (" << std::fixed << std::setprecision ( 2 ) << std::abs ( m_Speed_opt ) << " m/s)"
        << " Arrival Bottleneck "
        << t << " s"
        << std::endl;
    m_Arrival_Bottleneck = t;
}
void CPedestrian::ArrivalTransport ( double t )
{
    if ( PRINT )
        std::cout << "[" << ( m_Speed_opt > 0 ? "B" : "L" ) << "] "
        << m_ID << " (" << std::fixed << std::setprecision ( 2 ) << std::abs ( m_Speed_opt ) << " m/s)"
        << " Arrival Transport "
        << t << " s"
        << std::endl;
    m_Arrival_Transport = t;
}

double CPedestrian::NextUpdate ( void ) const
{
    return m_Next_Update;
}

void CPedestrian::NewUpdate ( double time )
{
    std::random_device rd;
    std::mt19937 gen ( rd() );
    while ( std::isinf( m_Next_Update = time + m_Exp_Distribution ( gen ) ) ); 
}
