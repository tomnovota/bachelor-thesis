#pragma once

#include <deque>
#include <memory>
#include <cmath>

#include "CPedestrian.h"

class CMass
{
  public:
    CMass ( void );

    virtual double GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const = 0;

    virtual double GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const = 0;

    virtual double GetMassSelf ( double pos, double from, double to ) const = 0;

    virtual std::string Name ( void ) const = 0;

    static bool DBL_eq ( double x, double y );

  protected:
};

class CDiraq : public CMass
{
  public:
    CDiraq ( void );

    virtual double GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const override;

    virtual double GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const override;

    virtual double GetMassSelf ( double pos, double from, double to ) const override;

    virtual std::string Name ( void ) const override;

  private:

};

class CFreeFlow : public CMass
{
  public:
    CFreeFlow ( double c_back, double c_front );

    virtual double GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const override;

    virtual double GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const override;

    virtual double GetMassSelf ( double pos, double from, double to ) const override;

    virtual std::string Name ( void ) const override;

  private:
    double m_c_back;
    double m_c_front;
};

class CTriangle : public CMass
{
  public:
    CTriangle ( double c_back, double c_front );

    double TriangContentBack ( double x ) const;

    double TriangContentFront ( double x ) const;

    virtual double GetMassB ( std::deque< std::shared_ptr< CPedestrian > > & activeB, double from, double to ) const override;

    virtual double GetMassL ( std::deque< std::shared_ptr< CPedestrian > > & activeL, double from, double to ) const override;

    virtual double GetMassSelf ( double pos, double from, double to ) const override;

    virtual std::string Name ( void ) const override;

  private:
    double m_c_back;
    double m_c_front;

    double m_Height;
    double m_Content_Back;
    double m_Content_Front;
};

