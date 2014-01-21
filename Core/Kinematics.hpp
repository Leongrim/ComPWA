//-------------------------------------------------------------------------------
// Copyright (c) 2013 Peter Weidenkaff.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//		Peter Weidenkaff -
//-------------------------------------------------------------------------------

#ifndef KINEMATICS_HPP_
#define KINEMATICS_HPP_

#include <vector>
#include <memory>

#include <boost/log/trivial.hpp>
using namespace boost::log;

#include "Core/DataPoint.hpp"
#include "Core/Event.hpp"
class dataPoint;

class Kinematics
{
public:
	static Kinematics* instance();
	const std::vector<std::string>& getVarNames(){return varNames;}
	//! checks of data point is within phase space boundaries
	virtual bool isWithinPhsp(const dataPoint& point) = 0;
	virtual double getMotherMass() = 0;
	virtual void eventToDataPoint(Event& ev, dataPoint& point) = 0;


protected:
	std::vector<std::string> varNames;
	static Kinematics* _inst;
	Kinematics() {};
	Kinematics(const Kinematics&) {};
	virtual ~Kinematics() {};
};

#endif /* KINEMATICS_HPP_ */
