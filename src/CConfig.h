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
#include "CParameters.h"

class CParameters;

class CConfig
{
  public:
    CConfig ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep );

    void Run ( size_t iter_cnt, double interval_max, CParameters & params );

    virtual void Simulate ( std::shared_ptr< CMass >, double, std::string ) = 0;

  protected:
    std::string m_Filename;
    std::vector< std::shared_ptr< CMass > > m_Kernels;
    double m_Timestep;

    std::vector< CPedestrian > m_WaitingB;
    std::vector< CPedestrian > m_WaitingL;
};

class CConfigTransportOnly : public CConfig
{
  public:
    CConfigTransportOnly ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep, std::shared_ptr< CTransport > t1 );

    virtual void Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename ) override;

  private:
    std::shared_ptr< CTransport > m_Transport1;
};

class CConfigAll : public CConfig
{
  public:
    CConfigAll ( std::string filename, std::vector< std::shared_ptr< CMass > > kernels, double timestep, std::shared_ptr< CTransport > t1, std::shared_ptr< CTransport > t2, std::shared_ptr< CBottleneck > b1, std::shared_ptr< CBottleneck > b2 );

    virtual void Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename ) override;

  private:
    std::shared_ptr< CTransport > m_Transport1; 
    std::shared_ptr< CTransport > m_Transport2; 
    std::shared_ptr< CBottleneck > m_BottleneckB;
    std::shared_ptr< CBottleneck > m_BottleneckL;
};

class CConfigBottleneckOnly : public CConfig
{
  public:
    virtual void Simulate ( std::shared_ptr< CMass > kernel, double interval_max, std::string filename ) override;

  private:
    std::shared_ptr< CBottleneck > m_Bottleneck1;
    std::shared_ptr< CBottleneck > m_Bottleneck2;
};

