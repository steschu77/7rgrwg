#pragma once

namespace filter {

class Filter
{
  public:
    Filter() {}
    Filter(Filter* pf0);
    Filter(Filter* pf0, Filter* pf1);
    Filter(Filter* pf0, Filter* pf1, Filter* pf2);
    Filter(Filter* pf0, Filter* pf1, Filter* pf2, Filter* pf3);

    virtual ~Filter() {}

    virtual double GetValue(double x, double y, double z) const = 0;

  protected:
    const Filter* _pSource[4] = { nullptr, nullptr, nullptr, nullptr };
    unsigned _cSources = 0;

  private:
    Filter(const Filter&) = delete;
    const Filter& operator = (const Filter&) = delete;

};

// ----------------------------------------------------------------------------
inline Filter::Filter(Filter* pf0)
{
  _pSource[0] = pf0;
  _cSources = 1;
}

// ----------------------------------------------------------------------------
inline Filter::Filter(Filter* pf0, Filter* pf1)
{
  _pSource[0] = pf0;
  _pSource[1] = pf1;
  _cSources = 2;
}

// ----------------------------------------------------------------------------
inline Filter::Filter(Filter* pf0, Filter* pf1, Filter* pf2)
{
  _pSource[0] = pf0;
  _pSource[1] = pf1;
  _pSource[2] = pf2;
  _cSources = 3;
}

// ----------------------------------------------------------------------------
inline Filter::Filter(Filter* pf0, Filter* pf1, Filter* pf2, Filter* pf3)
{
  _pSource[0] = pf0;
  _pSource[1] = pf1;
  _pSource[2] = pf2;
  _pSource[3] = pf3;
  _cSources = 4;
}

}
