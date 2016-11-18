#pragma once

#include "Filter.h"

namespace filter {

class Const: public Filter
{
public:
  Const(double Value=0.0): _constValue(Value) {}

  double GetValue(double, double, double) const {
    return _constValue;
  }

protected:
  double _constValue;
};

}
