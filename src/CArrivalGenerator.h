#pragma once

#include <queue>
#include <memory>
#include <vector>
#include <random>

class CArrivalGenerator
{
  public:
    static double CalcValue ( double time, const std::vector< double > & coeffs );

    static std::vector< double > GenerateEvents ( double interval, double lambda, std::vector< double > coeffs = {} );

  private:
};