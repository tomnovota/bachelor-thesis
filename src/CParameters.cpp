#include "CParameters.h"

CParameters::CParameters ( void ) = default;

std::shared_ptr< CConfig > CParameters::Read ( std::string filename )
{
    std::ifstream ifs ( filename );
    if ( ! ifs . is_open() )
        throw std::invalid_argument ( "Parameter file " + filename + " can't be open!\n" );

    ReadParams ( ifs );

    if ( ! m_Time_IntervalB && ! m_Time_IntervalL )
        ReadArrivals ( ifs );

    if ( m_LambdaB < 0 && ! ( CMass::DBL_eq ( m_LambdaB, 0 ) ) )
        ReadNonHomoB ( ifs );

    if ( m_LambdaL < 0 && ! ( CMass::DBL_eq ( m_LambdaL, 0 ) ) )
        ReadNonHomoL ( ifs );

    if ( ! m_Distance_Bottleneck )
        return OnlyTransport ( ifs );

    if ( ! m_Distance_Transport1 )
        return OnlyBottleneck (  );

    auto transport1 = std::make_shared< CTransportB > ( 0, m_Distance_Transport1, m_Timestep, m_R_crit, m_R_max );

    auto bottleneck1 = std::make_shared< CBottleneckB > ( m_Distance_Bottleneck, m_Body_Width );
    auto bottleneck2 = std::make_shared< CBottleneckL > ( m_Distance_Bottleneck, m_Body_Width );

    auto transport2 = std::make_shared< CTransportL > ( m_Distance_Transport1 + m_Distance_Bottleneck, Distance(), m_Timestep, m_R_crit, m_R_max );

    return std::make_shared< CConfigAll > ( m_Filename, GetKernels(), m_Timestep, transport1, transport2, bottleneck1, bottleneck2 );
}

void CParameters::ReadNonHomoB ( std::ifstream & ifs )
{
    std::string line;
    std::getline ( ifs, line );
    std::stringstream ss ( line );

    m_LambdaB = std::abs ( m_LambdaB );

    double tmp;
    while ( ss >> tmp )
    {
        m_PCoeffsB . push_back ( tmp );
    }
}

void CParameters::ReadNonHomoL ( std::ifstream & ifs )
{
    std::string line;
    std::getline ( ifs, line );
    std::stringstream ss ( line );

    m_LambdaL = std::abs ( m_LambdaL );

    double tmp;
    while ( ss >> tmp )
    {
        m_PCoeffsL . push_back ( tmp );
    }
}

std::shared_ptr< CConfig > CParameters::OnlyTransport ( std::ifstream & ifs )
{
    std::deque< std::shared_ptr< CPedestrian > > activeB;
    std::deque< std::shared_ptr< CPedestrian > > activeL;
    ReadArtificial ( ifs, activeB, activeL );

    auto transport1 = std::make_shared< CTransportB > ( 0, m_Distance_Transport1, m_Timestep, m_R_crit, m_R_max, activeB, activeL );

    return std::make_shared< CConfigTransportOnly > ( m_Filename, GetKernels(), m_Timestep, transport1 );
}

std::shared_ptr< CConfig > CParameters::OnlyBottleneck ( void )
{
    return nullptr;
}

std::vector< std::shared_ptr< CMass > > CParameters::GetKernels ( void ) const
{
    switch ( m_Kernels )
    {
    case 0:
        return std::vector< std::shared_ptr< CMass > >
        {
            std::make_shared< CDiraq > ()
            , std::make_shared< CTriangle > ( m_c_back, m_c_front )
            , std::make_shared< CTriangle > ( m_c_front, m_c_back )
            , std::make_shared< CFreeFlow > ( m_c_back, m_c_front )
        };
    case 1:
        return std::vector< std::shared_ptr< CMass > >
        {
            std::make_shared< CDiraq > ()
        };
    case 2:
        return std::vector< std::shared_ptr< CMass > >
        {
            std::make_shared< CTriangle > ( m_c_back, m_c_front )
        };
    case 3:
        return std::vector< std::shared_ptr< CMass > >
        {
            std::make_shared< CTriangle > ( m_c_front, m_c_back )
        };
    case 4:
        return std::vector< std::shared_ptr< CMass > >
        {
            std::make_shared< CFreeFlow > ( m_c_back, m_c_front )
        };
    }
    return {};
}

double CParameters::GetSpeed ( double coeff ) const
{
    double speed;
    if ( CMass::DBL_eq ( m_Speed_trim, 0 ) )
        return coeff * m_Speed_med;

    std::normal_distribution<> normal_d { m_Speed_med, m_Speed_std };
    std::random_device rd;
    std::mt19937 gen ( rd() );
    while ( std::abs ( ( speed = normal_d ( gen ) ) - m_Speed_med ) > std::abs ( m_Speed_trim ) );
    return coeff * speed;
}

void CParameters::ReadArtificial ( std::ifstream & ifs, std::deque< std::shared_ptr< CPedestrian > > & activeB, std::deque< std::shared_ptr< CPedestrian > > & activeL )
{
    std::string line;
    std::getline ( ifs, line ); // skip empty line
    while ( std::getline ( ifs, line )
            && ! line . empty() )
    {
        double position;
        size_t id, type;
        std::istringstream iss ( line );
        if ( ! ( iss >> id >> position >> type ) )
            throw std::invalid_argument ( "Reading artifical pedestrians error\n" );

        if ( type == 0 )
        {
            activeB . push_back ( std::make_shared< CPedestrian > ( id, 0, position, m_d1_dir, m_d2_dir, m_d1_op, m_d2_op, 0, m_Body_Width ) );
        }
        else
        {
            activeL . push_back ( std::make_shared< CPedestrian > ( id, 0, position, m_d1_dir, m_d2_dir, m_d1_op, m_d2_op, 0, m_Body_Width ) );
        }
    }
}

void CParameters::ReadArrivals ( std::ifstream & ifs )
{
    std::string line;
    std::getline ( ifs, line ); // skip empty line
    while ( std::getline ( ifs, line )
            && ! line . empty() )
    {
        double speed, departure;
        size_t id;
        std::istringstream iss ( line );
        if ( ! ( iss >> id >> speed >> departure ) )
            throw std::invalid_argument ( "Reading pedestrians error\n" );

        if ( speed > 0 )
        {
            m_WaitingB . push_back ( CPedestrian ( id, departure, 0, m_d1_dir, m_d2_dir, m_d1_op, m_d2_op, speed, m_Body_Width ) );
        }
        else
        {
            m_WaitingL . push_back ( CPedestrian ( id, departure, Distance(), m_d1_dir, m_d2_dir, m_d1_op, m_d2_op, speed, m_Body_Width ) );
        }
    }
}

std::vector< CPedestrian > CParameters::GenerateArrivalsB ( void )
{
    if ( ! m_Time_IntervalB && ! m_Time_IntervalL )
        return m_WaitingB;

    std::vector< CPedestrian > waiting;
    auto events = CArrivalGenerator::GenerateEvents ( m_Time_IntervalB, m_LambdaB, m_PCoeffsB );
    for ( size_t id = 0; id < events . size(); ++ id )
    {
        waiting . push_back ( CPedestrian ( id, events [ id ], 0, m_d1_dir, m_d2_dir, m_d1_op, m_d2_op, GetSpeed ( 1 ), m_Body_Width ) );
    }
    return waiting;
}

std::vector< CPedestrian > CParameters::GenerateArrivalsL ( void )
{
    if ( ! m_Time_IntervalB && ! m_Time_IntervalL )
        return m_WaitingL;

    std::vector< CPedestrian > waiting;
    auto events = CArrivalGenerator::GenerateEvents ( m_Time_IntervalL, m_LambdaL, m_PCoeffsL );
    for ( size_t id = 0; id < events . size(); ++ id )
    {
        waiting . push_back ( CPedestrian ( id, events [ id ], Distance(), m_d1_dir, m_d2_dir, m_d1_op, m_d2_op,
        ( m_Speed_trim < 0 ? - m_Speed_med : GetSpeed ( - 1 ) ), m_Body_Width ) );
    }
    return waiting;
}


void CParameters::ReadParams ( std::ifstream & ifs )
{
    std::size_t cnt = 0;
    if ( ! ( ReadParam ( ifs, m_Filename ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Timestep ) && ( ++ cnt )

            &&  ReadParam ( ifs, m_Body_Width ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Speed_med ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Speed_std ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Speed_trim ) && ( ++ cnt )

            &&  ReadParam ( ifs, m_R_crit ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_R_max ) && ( ++ cnt )

            &&  ReadParam ( ifs, m_d1_dir ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_d2_dir ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_d1_op ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_d2_op ) && ( ++ cnt )

            &&  ReadParam ( ifs, m_c_back ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_c_front ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Distance_Transport1 ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Distance_Bottleneck ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Distance_Transport2 ) && ( ++ cnt )

            &&  ReadParam ( ifs, m_Time_IntervalB ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Time_IntervalL ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_LambdaB ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_LambdaL ) && ( ++ cnt )
            &&  ReadParam ( ifs, m_Kernels ) && ( ++ cnt )
    ) ) throw std::invalid_argument
    ( "Reading parameter error [" + std::to_string ( cnt ) + "]\n" );
}

template< typename T_ >
int CParameters::ReadParam ( std::ifstream & ifs, T_ & param )
{
    std::string line;
    std::istringstream iss;
    return std::getline ( ifs, line )
        && ( iss . str ( line ), ( iss >> param ) );
}

double CParameters::Distance ( void ) const
{
    return m_Distance_Transport1 + m_Distance_Bottleneck + m_Distance_Transport2;
}