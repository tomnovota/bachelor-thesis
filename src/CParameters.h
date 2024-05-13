#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <exception>
#include <iomanip>
#include <memory>
#include <deque>
#include <queue>
#include <cmath>
#include <thread>
#include <filesystem>

#include "CTransport.h"
#include "CBottleneck.h"
#include "CPedestrian.h"
#include "CArrivalGenerator.h"
#include "CMass.h"
#include "CConfig.h"

class CConfig;

class CParameters
{
  public:
    CParameters ( void );

    std::shared_ptr< CConfig > Read ( std::string filename );

    std::shared_ptr< CConfig > OnlyTransport ( std::ifstream & ifs );

    std::shared_ptr< CConfig > OnlyBottleneck ( void );

    std::vector< std::shared_ptr< CMass > > GetKernels ( void ) const;

    double GetSpeed ( double coeff ) const;

    std::vector< CPedestrian > GenerateArrivalsB ( void );
    std::vector< CPedestrian > GenerateArrivalsL ( void );

    void ReadArtificial ( std::ifstream & ifs, std::deque< std::shared_ptr< CPedestrian > > & activeB, std::deque< std::shared_ptr< CPedestrian > > & activeL );

    void ReadArrivals ( std::ifstream & ifs );

    void ReadNonHomoB ( std::ifstream & ifs );
    void ReadNonHomoL ( std::ifstream & ifs );

    void ReadParams ( std::ifstream & ifs );

    template< typename T_ >
    int ReadParam ( std::ifstream & ifs, T_ & param );

    double Distance ( void ) const;

  private:
    std::string m_Filename;
    double m_Timestep = 0.1;

    double m_Body_Width = 0.75;
    double m_Speed_med = 1.2;
    double m_Speed_std = 0.26;
    double m_Speed_trim = 0.6;

    double m_R_crit = 6;
    double m_R_max = 14;

    double m_d1_dir = 0;
    double m_d2_dir = 5;
    double m_d1_op = 0;
    double m_d2_op = 5;

    double m_c_back = 1;
    double m_c_front = 1;

    double m_Distance_Transport1 = 300;
    double m_Distance_Bottleneck = 0;
    double m_Distance_Transport2 = 0;

    double m_Time_IntervalB = 1200;
    double m_Time_IntervalL = 1200;
    double m_LambdaB = 0.33;
    double m_LambdaL = 0.33;

    std::vector< double > m_PCoeffsB;
    std::vector< double > m_PCoeffsL;

    int m_Kernels = 0;

    std::vector< CPedestrian > m_WaitingL;
    std::vector< CPedestrian > m_WaitingB;
};