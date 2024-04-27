#pragma once

#include <fstream>
#include <iomanip>
#include <cmath>
#include <random>
#include <memory>
#include <iostream>

class CPedestrian
{
  public:
    CPedestrian ( size_t id, double departure, double position, double d1_dir, double d2_dir, double d1_op, double d2_op, double speed, double body );

    double GetArrival ( void ) const;
    
    double GetPosition ( void ) const;

    std::size_t GetID ( void ) const;

    void SetPosition ( double pos );

    void WriteTransport ( std::shared_ptr< std::ofstream > ofs, double coeff );

    void WriteEnd ( std::shared_ptr< std::ofstream > ofs, double time );

    void WriteBottleneck ( std::shared_ptr< std::ofstream > ofs, std::size_t pos );

    double FromDir ( void ) const;

    double ToDir ( void ) const;

    double FromOp ( void ) const;

    double ToOp ( void ) const;

    void UpdatePosition ( double coef );

    void ArrivalBottleneck ( double t );

    void ArrivalTransport ( double t );

    double NextUpdate ( void ) const;

    void NewUpdate ( double time );

  private:
    std::size_t m_ID;
    double m_Speed_opt;

    double m_Arrival_Start;
    double m_Arrival_Bottleneck;
    double m_Arrival_Transport;

    double m_Position_Curr;

    double m_d1_dir;
    double m_d2_dir;
    double m_d1_op;
    double m_d2_op;

    double m_Next_Update = 0;
    std::exponential_distribution<> m_Exp_Distribution;
};