#include "OIFMinuitFcn.hpp"
#include "OIFData.hpp"
//#include "ErrLogger/ErrLogger.hh"
#include <cassert>
#include <memory>
#include <iostream>

using namespace ROOT::Minuit2;

OIFMinuitFcn::OIFMinuitFcn(std::shared_ptr<OIFData> myData) :
  _myDataPtr(myData)
{
  if (0==_myDataPtr) {
    //Alert << "Data pointer is 0 !!!!" << endmsg;
      std::cout << "Data pointer is 0 !!!!" << std::endl;
    exit(1);
  }
}

OIFMinuitFcn::~OIFMinuitFcn()
{
}

double OIFMinuitFcn::operator()(const std::vector<double>& par) const
{
  double result=_myDataPtr->controlParameter(par);
  //DebugMsg << "current minimized value:\t"<< result << endmsg;
  std::cout << "current minimized value:\t"<< result << std::endl;
  return result;
}

double OIFMinuitFcn::Up() const
{
return 1.;
}



