#pragma once

#include <vector>
#include <deque>
#include <queue>
#include <memory>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <exception>

#include "CTransport.h"
#include "CPedestrian.h"

class CTransport;

class CBottleneck
{
  public:
    CBottleneck ( double distance, double body_width );

    bool Finished ( void ) const;

    void Arrival ( std::shared_ptr< CPedestrian > ped, double time );

    void Departure ( double time );

    void AssignTransport ( std::shared_ptr< CTransport > transport );

    void AssignUpdateTime ( double time, std::size_t position );

    void Step ( double t_n );

    void Update ( double time, std::size_t position );

    void PlaceInOrder ( double time, std::size_t position );

    virtual void Log ( double time ) = 0;

    virtual std::shared_ptr< CBottleneck > CreateInstance ( std::string filename ) const = 0;

  protected:
    double m_Distance;

    std::vector< std::shared_ptr< CPedestrian > > m_Path;
    std::queue< std::shared_ptr< CPedestrian > > m_Queue;
    std::deque< std::size_t > m_Update_Order;

    std::weak_ptr< CTransport > m_Transport_Out;

    std::shared_ptr< std::ofstream > m_ofstream;
};

class CBottleneckB : public CBottleneck
{
  public:
    CBottleneckB ( double distance, double body_width );

    virtual std::shared_ptr< CBottleneck > CreateInstance ( std::string filename ) const override;

    virtual void Log ( double time ) override;

};

class CBottleneckL : public CBottleneck
{
  public:
    CBottleneckL ( double distance, double body_width );

    virtual std::shared_ptr< CBottleneck > CreateInstance ( std::string filename ) const override;

    virtual void Log ( double time ) override;

};