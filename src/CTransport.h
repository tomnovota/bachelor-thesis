#pragma once

#include <queue>
#include <deque>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <exception>

#include "CBottleneck.h"
#include "CMass.h"
#include "CPedestrian.h"

class CBottleneck;

class CTransport
{
  public:
    CTransport ( double start, double end, double timestep, double r_crit, double r_max, std::deque< std::shared_ptr< CPedestrian > > activeB = {}, std::deque< std::shared_ptr< CPedestrian > > activeL = {} );

    double FD ( double mass );

    bool Finished ( void ) const;

    void AssignBottleneck ( std::shared_ptr< CBottleneck > bottleneck );

    void SetWaiting ( const std::vector< CPedestrian > & waiting );

    void Step ( double t_n );

    void Sort ( void );

    void Premature ( double time );

    virtual void Arrival ( std::shared_ptr< CPedestrian > ped ) = 0;

    virtual void Departure ( double t_n ) = 0;

    virtual std::shared_ptr< CTransport > CreateInstance ( std::string name, std::shared_ptr< CMass > mass ) const = 0;

    static bool WRITE_TRAJECT;

  protected:
    double m_Start;
    double m_End;
    double m_Timestep;

    double m_R_Crit;
    double m_R_Max;

    std::deque< std::shared_ptr< CPedestrian > > m_ActiveB;
    std::deque< std::shared_ptr< CPedestrian > > m_ActiveL;

    std::queue< std::shared_ptr< CPedestrian > > m_Waiting;

    std::shared_ptr< CMass > m_Mass = nullptr;
    std::weak_ptr< CBottleneck > m_Bottleneck_Out;

    std::shared_ptr< std::ofstream > m_ofstream_trajectory;
    std::shared_ptr< std::ofstream > m_ofstream_end;
    size_t m_Static_Count;
};

class CTransportB : public CTransport
{
  public:
    CTransportB ( double start, double end, double timestep, double r_crit, double r_max, std::deque< std::shared_ptr< CPedestrian > > activeB = {}, std::deque< std::shared_ptr< CPedestrian > > activeL = {} );

    virtual void Arrival ( std::shared_ptr< CPedestrian > ped ) override;

    virtual void Departure ( double t_n ) override;

    virtual std::shared_ptr< CTransport > CreateInstance ( std::string name, std::shared_ptr< CMass > mass ) const override;

  private:

};

class CTransportL : public CTransport
{
  public:
    CTransportL ( double start, double end, double timestep, double r_crit, double r_max );

    void Arrival ( std::shared_ptr< CPedestrian > ped );

    void Departure ( double t_n );

    std::shared_ptr< CTransport > CreateInstance ( std::string name, std::shared_ptr< CMass > mass ) const;

  private:

};