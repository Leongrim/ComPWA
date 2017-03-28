#ifndef TWOBODYKIN
#define TWOBODYKIN

#include "Core/Kinematics.hpp"

namespace ComPWA {

class dataPoint;

class TwoBodyKinematics : public Kinematics {
public:
  void init();

  static Kinematics *createInstance(int idMother, std::vector<int> finalState,
                  double deltaMassWindow = 0.5) {
    if( _inst )
      throw std::runtime_error("TwoBodyKinematics::createInstance() | Instance already exists!");
    _inst = new TwoBodyKinematics(idMother, finalState, deltaMassWindow );
    return _inst;
  }

  //! Converts Event to dataPoint
  virtual void EventToDataPoint(const Event &ev, dataPoint &point) const;

  virtual void FillDataPoint(int a, int b, double invMassSqA, double invMassSqB,
                             dataPoint &point) const {};

  //! checks of data point is within phase space boundaries
  virtual bool IsWithinPhsp(const dataPoint &point) const;

  /**! Checks if the position is within the phase-space boundaries.
   * This only works correctly if both variables are orthogonal to each other.
   * E.g. and invariant mass and an angle.
   * @param idA Variable id of invariant mass
   * @param idB Variable id of angle
   * @param varA Invariant mass
   * @param varB Helicity angle
   * @return
   */
  virtual bool IsWithinBoxPhsp(int idA, int idB, double varA,
                               double varB) const {
    return 0;
  };
  virtual unsigned int GetNVars() const { return 1; }

  //! get mass of particles
  virtual double GetMass(unsigned int num) const;

protected:
  
  TwoBodyKinematics(int idMother, std::vector<int> finalState,
                  double deltaMassWindow);
  
  double _M;
  ComPWA::Spin _spinM;
  double mSq1;        //! masse squared of daughter 1
  double m1;          //! masses of daughter 1
  unsigned int spin1; //! spin of daughter 1

  double mSq2;        //! masse squared of daughter 2
  double m2;          //! masses of daughter 2
  unsigned int spin2; //! spin of daughter 2

  double mass_sq_min; //! minimum value of masssq
  double mass_sq_max; //! maximum value of masssq
  double mass_min;    //! minimum value of masssq
  double mass_max;    //! maximum value of masssq

  virtual double calculatePSArea() { return (mass_max - mass_min); }
};

} /* namespace ComPWA */
#endif /* TWOBODYKINEMATICS_HPP_ */
