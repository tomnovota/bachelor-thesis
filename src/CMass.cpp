#include "CMass.h"

bool CMass::DBL_eq ( double x, double y )
{
    return std::abs ( x - y ) <= 10e-12 * ( std::abs ( x ) + std::abs ( y ) );
}


CMass::CMass ( void ) = default;

CDiraq::CDiraq ( void ) = default;

double CDiraq::GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const
{
    double mass = 0;
    for ( auto ped : activeB )
    {
        if ( ( from < ped -> GetPosition() || DBL_eq ( from, ped -> GetPosition() ) )
          && ( to > ped -> GetPosition() || DBL_eq ( to, ped -> GetPosition() ) )
        )
            mass += 1;
    }
    return mass;
}

double CDiraq::GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const
{
    double mass = 0;
    for ( auto ped : activeL )
    {
        if ( ( from < ped -> GetPosition() || DBL_eq ( from, ped -> GetPosition() ) )
          && ( to > ped -> GetPosition() || DBL_eq ( to, ped -> GetPosition() ) )
        )
            mass += 1;
    }
    return mass;
}

double CDiraq::GetMassSelf ( double pos, double from, double to ) const
{
    return ( ( from < pos || DBL_eq ( from, pos ) )
          && ( to > pos || DBL_eq ( to, pos ) )
        )
        ? 1
        : 0;
}

std::string CDiraq::Name ( void ) const
{
    return "diraq";
}

CFreeFlow::CFreeFlow ( double c_back, double c_front )
: m_c_back ( c_back )
, m_c_front ( c_front )
{}

double CFreeFlow::GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const
{
    return 0;
}

double CFreeFlow::GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const
{
    return 0;
}

double CFreeFlow::GetMassSelf ( double pos, double from, double to ) const
{
    return 0;
}

std::string CFreeFlow::Name ( void ) const
{
    return "free";
}

CTriangle::CTriangle ( double c_back, double c_front )
: m_c_back ( c_back )
, m_c_front ( c_front )
, m_Height ( 2 / ( c_back + c_front ) )
, m_Content_Back ( c_back * m_Height / 2 )
, m_Content_Front ( c_front * m_Height / 2 )
{}

double CTriangle::TriangContentBack ( double x ) const
{
    double height = m_Height * ( 1 - std::abs ( x ) / m_c_back );
    double side = std::max ( 0., m_c_back - std::abs ( x ) );

    return height * side / 2;
}

double CTriangle::TriangContentFront ( double x ) const
{
    double height = m_Height * ( 1 - std::abs ( x ) / m_c_front );
    double side = std::max ( 0., m_c_front - std::abs ( x ) );

    return height * side / 2;
}

double CTriangle::GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const
{
    double mass = 0, left, right;
    for ( auto ped : activeB )
    {
        left = from - ped -> GetPosition();
        right = to - ped -> GetPosition();
        if ( right <= - m_c_back || left >= m_c_front )
            continue;

        if ( right < 0 ) // left small
        {
            mass += TriangContentBack ( right );
        }
        else if ( left > 0 ) // right small
        {
            mass += TriangContentFront ( left );
        }
        else if ( left <= - m_c_back ) // full left
        {
            mass += m_Content_Back + m_Content_Front;
            if ( right < m_c_front )
            {
                mass -= TriangContentFront ( right ); // right trim
            }
        }
        else if ( right >= m_c_front ) // full right
        {
            mass += m_Content_Back + m_Content_Front;
            if ( left > - m_c_back )
            {
                mass -= TriangContentBack ( left ); // left trim
            }
        }
    }
    return mass;
}

double CTriangle::GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const
{
    double mass = 0, left, right;
    for ( auto ped : activeL )
    {
        left = from - ped -> GetPosition();
        right = to - ped -> GetPosition();
        if ( right <= - m_c_front || left >= m_c_back )
            continue;

        if ( right < 0 ) // left small
        {
            mass += TriangContentFront ( right );
        }
        else if ( left > 0 ) // right small
        {
            mass += TriangContentBack ( left );
            continue;
        }
        else if ( left <= - m_c_front ) // full left
        {
            mass += m_Content_Back + m_Content_Front;
            if ( right < m_c_back )
            {
                mass -= TriangContentBack ( right ); // right trim
            }
        }
        else if ( right >= m_c_back ) // full right
        {
            mass += m_Content_Back + m_Content_Front;
            if ( left > - m_c_front )
            {
                mass -= TriangContentFront ( left ); // left trim
            }
        }
    }
    return mass;
}

double CTriangle::GetMassSelf ( double pos, double from, double to ) const
{
    double mass = 0, left, right;
    left = std::abs ( from - pos );
    right = std::abs ( to - pos );

    if ( DBL_eq ( std::min ( left, right ), 0 ) )
        return m_Content_Front;

    return mass + TriangContentFront ( std::min ( left, right ) );
}

std::string CTriangle::Name ( void ) const
{
    return m_c_back < m_c_front
        ? "triang1"
        : "triang2";
}

