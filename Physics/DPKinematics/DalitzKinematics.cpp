//-------------------------------------------------------------------------------
// Copyright (c) 2013 Peter Weidenkaff.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Peter Weidenkaff - initial API
//     Mathias Michel - kinematic functions
//-------------------------------------------------------------------------------

#include <stdlib.h>
#include "Physics/DPKinematics/DalitzKinematics.hpp"
//#include "Physics/DPKinematics/DataPoint.hpp"
#include "Core/PhysConst.hpp"
#include "Core/DataPoint.hpp"

#include <stdlib.h>
#include "gsl/gsl_monte.h"
#include "gsl/gsl_monte_plain.h"
#include "gsl/gsl_monte_miser.h"
#include "gsl/gsl_monte_vegas.h"
#include "gsl/gsl_sf_legendre.h"


DalitzKinematics::DalitzKinematics(std::string _nameMother,
		std::string _name1, std::string _name2, std::string _name3) :
		Br(0.0), nameMother(_nameMother),
		name1(_name1), name2(_name2), name3(_name3), massIdsSet(false)
{
	nPart = 3;
	M = PhysConst::instance()->getMass(_nameMother);
	m1 = PhysConst::instance()->getMass(_name1);
	m2 = PhysConst::instance()->getMass(_name2);
	m3 = PhysConst::instance()->getMass(_name3);
	spinM = PhysConst::instance()->getJ(_nameMother);
	spin1 = PhysConst::instance()->getJ(_name1);
	spin2 = PhysConst::instance()->getJ(_name2);
	spin3 = PhysConst::instance()->getJ(_name3);

	if(M==-999 || m1==-999|| m2==-999|| m3==-999) {
		BOOST_LOG_TRIVIAL(error)<<"Masses not set! EXIT!";
		exit(1);
	}
	BOOST_LOG_TRIVIAL(info) << " DalitzKinematics::DalitzKinematics() | Setting up decay "
			<<_nameMother<<"->"<<_name1<<" "<<_name2<<" "<<_name3;
	init();
}

DalitzKinematics::DalitzKinematics(double _M, double _Br,
		double _m1, double _m2, double _m3,
		std::string _nameMother, std::string _name1,
		std::string _name2, std::string _name3) :
			M(_M), Br(_Br), m1(_m1), m2(_m2), m3(_m3),
			nameMother(_nameMother), name1(_name1), name2(_name2),
			name3(_name3), massIdsSet(false)
{
	nPart = 3;
	spinM = PhysConst::instance()->getJ(_nameMother);
	spin1 = PhysConst::instance()->getJ(_name1);
	spin2 = PhysConst::instance()->getJ(_name2);
	spin3 = PhysConst::instance()->getJ(_name3);
	init();
}

void DalitzKinematics::init()
{
	Msq = M*M;
	mSq1 = m1*m1;
	mSq2 = m2*m2;
	mSq3 = m3*m3;
	mSq4 = m4*m4;

	_DPareaCalculated=0;

	varNames.push_back("m23sq");
	varNames.push_back("m13sq");
	varNames.push_back("m12sq");
	// Angle between particle 1 and 3 in the rest frame of 1 and 2
	varNames.push_back("cosTheta12_13");
	// Angle between particle 2 and 3 in the rest frame of 1 and 2
	varNames.push_back("cosTheta12_23");
	// Angle between particle 1 and 2 in the rest frame of 1 and 3
	varNames.push_back("cosTheta13_12");
	// Angle between particle 2 and 3 in the rest frame of 1 and 3
	varNames.push_back("cosTheta13_23");
	// Angle between particle 1 and 3 in the rest frame of 2 and 3
	varNames.push_back("cosTheta23_13");
	// Angle between particle 1 and 2 in the rest frame of 2 and 3
	varNames.push_back("cosTheta23_12");

	varTitles.push_back("m_{"+name2+name3+"}^{2} [ GeV^{2}/c^{4} ]");
	varTitles.push_back("m_{"+name1+name3+"}^{2} [ GeV^{2}/c^{4} ]");
	varTitles.push_back("m_{"+name1+name2+"}^{2} [ GeV^{2}/c^{4} ]");
	varTitles.push_back("cos(#Theta^{"+name1+name2+"}_{"+name1+name3+"})");
	varTitles.push_back("cos(#Theta^{"+name1+name2+"}_{"+name2+name3+"})");
	varTitles.push_back("cos(#Theta^{"+name1+name3+"}_{"+name1+name2+"})");
	varTitles.push_back("cos(#Theta^{"+name1+name3+"}_{"+name2+name3+"})");
	varTitles.push_back("cos(#Theta^{"+name2+name3+"}_{"+name1+name3+"})");
	varTitles.push_back("cos(#Theta^{"+name2+name3+"}_{"+name1+name2+"})");

	auto m23sq_limit = GetMinMax(0);
	auto m13sq_limit = GetMinMax(1);
	auto m12sq_limit = GetMinMax(2);
	BOOST_LOG_TRIVIAL(debug) << "PHSP boundaries:";
	BOOST_LOG_TRIVIAL(debug) << m23sq_limit.first<<" < m23sq < "<<m23sq_limit.second;
	BOOST_LOG_TRIVIAL(debug) << m13sq_limit.first<<" < m13sq < "<<m13sq_limit.second;
	BOOST_LOG_TRIVIAL(debug) << m12sq_limit.first<<" < m12sq < "<<m12sq_limit.second;

	return;
}

unsigned int DalitzKinematics::findVariable(std::string varName) const
{
	for(unsigned int i=0; i< varNames.size(); i++)
		if(varNames.at(i)==varName) return i;
	return -999;
}

double DalitzKinematics::calculateMoments(unsigned int sys, dataPoint& point,
		unsigned int n, unsigned int m)
{

	return gsl_sf_legendre_sphPlm(n,m,point.getVal(sys)); //normalized
}

void DalitzKinematics::eventToDataPoint(Event& ev, dataPoint& point)
{
	double weight = ev.getWeight();
	point.setWeight(weight);//reset weight
	Particle part1 = ev.getParticle(0);
	Particle part2 = ev.getParticle(1);
	Particle part3 = ev.getParticle(2);
	double m23sq = Particle::invariantMass(part2,part3);
	double m13sq = Particle::invariantMass(part1,part3);
	double m12sq = Particle::invariantMass(part1,part2);
	//double m12sq = getThirdVariableSq(m23sq,m13sq);

	point.setVal(0,m23sq);
	point.setVal(1,m13sq);
	point.setVal(2,m12sq);

	point.setVal(3,helicityAngle(M,m1,m2,m3,m12sq,m13sq));
	point.setVal(4,helicityAngle(M,m2,m1,m3,m12sq,m23sq));
	point.setVal(5,helicityAngle(M,m1,m3,m2,m13sq,m12sq));
	point.setVal(6,helicityAngle(M,m3,m1,m2,m13sq,m23sq));
	point.setVal(7,helicityAngle(M,m3,m2,m1,m23sq,m13sq));
	point.setVal(8,helicityAngle(M,m2,m3,m1,m23sq,m12sq));

	return;
}
void DalitzKinematics::FillDataPoint(int a, int b,
		double invMassSqA, double invMassSqB, dataPoint& point)
{
	if( a != 0 && a != 1 && a != 2){
		std::cout<<"A "<<a<<" "<<b<<" "<<invMassSqA<<std::endl;
		throw std::runtime_error("DalitzKinematics::FillDataPoint() | "
				"Particle ID out of range: "+std::to_string(a));
	}
	if( b != 0 && b != 1 && b != 2){
		std::cout<<"B "<<a<<" "<<b<<" "<<invMassSqA<<std::endl;
		throw std::runtime_error("DalitzKinematics::FillDataPoint() | "
				"Particle ID out of range: "+std::to_string(b));
	}
	int c;
	if( (a==0 && b==1) || (a==1 && b==0) ) c=2;
	if( (a==0 && b==2) || (a==2 && b==0) ) c=1;
	if( (a==1 && b==2) || (a==2 && b==1) ) c=0;
	point.setVal(a,invMassSqA);
	point.setVal(b,invMassSqB);
	point.setVal(c,getThirdVariableSq(invMassSqA,invMassSqB));

	double m23sq = point.getVal(0);
	double m13sq = point.getVal(1);
	double m12sq = point.getVal(2);

	point.setVal(3,helicityAngle(M,m1,m2,m3,m12sq,m13sq));
	point.setVal(4,helicityAngle(M,m2,m1,m3,m12sq,m23sq));
	point.setVal(5,helicityAngle(M,m1,m3,m2,m13sq,m12sq));
	point.setVal(6,helicityAngle(M,m3,m1,m2,m13sq,m23sq));
	point.setVal(7,helicityAngle(M,m3,m2,m1,m23sq,m13sq));
	point.setVal(8,helicityAngle(M,m2,m3,m1,m23sq,m12sq));

	return;
}

std::pair<double,double> DalitzKinematics::GetMinMaxLocal(unsigned int id_massA,
		unsigned int id_massB, double massA) const
{
	unsigned int part1, part2;
	double Mfirst, Msecond;
	switch(id_massA){
	case 0: part1=2; part2=3; Mfirst=m2; Msecond=m3; break;
	case 1: part1=3; part2=1; Mfirst=m3; Msecond=m1; break;
	case 2: part1=1; part2=2; Mfirst=m1; Msecond=m2; break;
	}
	double Efirst = eiCms(part1, id_massB, massA);
	double pFirst = sqrt( Efirst*Efirst - Mfirst*Mfirst );
	double Esecond = eiCms(part2, id_massB, massA);
	double pSecond = sqrt( Esecond*Esecond - Msecond*Msecond );

	std::pair<double,double> r(
			std::pow( Efirst + Esecond,2) - std::pow( pFirst + pSecond ,2),
			std::pow( Efirst + Esecond,2) - std::pow( pFirst - pSecond ,2)
	);
	return r;
}

double DalitzKinematics::eiCms(unsigned int partId,
		unsigned int id_mass, double invMassSq) const
{
	double E;
	double m = sqrt(invMassSq);
	switch(id_mass){
	case 2:
		switch(partId){
		case 1: E = (invMassSq-mSq2+mSq1)/(2*m);break;
		case 2: E = (invMassSq-mSq1+mSq2)/(2*m);break;
		case 3: E =-(invMassSq-Msq+mSq3)/(2*m);break;
		}
		break;
		case 1:
			switch(partId){
			case 1: E = (invMassSq-mSq3+mSq1)/(2*m);break;
			case 2: E =-(invMassSq-Msq+mSq2)/(2*m);break;
			case 3: E = (invMassSq-mSq1+mSq3)/(2*m); break;
			}
			break;
			case 0:
				switch(partId){
				case 1: E =-(invMassSq-Msq+mSq1)/(2*m);break;
				case 2: E = (invMassSq-mSq3+mSq2)/(2*m);break;
				case 3: E = (invMassSq-mSq2+mSq3)/(2*m); break;
				}
				break;
	}

	return E;
}

std::pair<double,double> DalitzKinematics::GetMinMax(std::string varName) const
{
	return GetMinMax(findVariable(varName));
}

std::pair<double,double> DalitzKinematics::GetMinMax(unsigned int i) const
{
	if(i >= GetNVars() )
		throw WrongVariableID("DalitzKinematics::GetMinMax() | "
				"Wrong variable ID "+std::to_string(i)+"!");

	//Limits for invariant masses
	switch (i)
	{
	case 0:  return std::make_pair<double,double>(
			(m2+m3)*(m2+m3),
			(M-m1)*(M-m1)
	);
	break;
	case 1: return std::make_pair<double,double>(
			(m1+m3)*(m1+m3),
			(M-m2)*(M-m2)
	);
	break;
	case 2:  return std::make_pair<double,double>(
			(m1+m2)*(m1+m2),
			(M-m3)*(M-m3)
	);
	break;
	}

	//Limits for helicity angles
	return std::make_pair<double,double>(-1,1);
}

void DalitzKinematics::phspContour(unsigned int xsys,unsigned int ysys,
		unsigned int n, double* xcoord, double* ycoord)
{
	if(xsys >= GetNVars() || ysys >= GetNVars())
		throw WrongVariableID("DalitzKinematics::phspContour() | "
				"Wrong variable ID!");

	unsigned int num=n;
	if(num%2!=0) {
		num-=1;
		BOOST_LOG_TRIVIAL(info)<<"DalitzKinematics::phspContour() | "
				"Setting size to a even number. Assure that the size of "
				"your arrays is "<<num*2+1<<"!";
	}

	std::pair<double,double> xlimits = GetMinMax(xsys);
	double binw = (xlimits.second-xlimits.first)/(double)(num);
	double ymin,ymax,x;
	unsigned int i=0;

	for (; i<num; i++)
	{
		x = i*binw + xlimits.first;
		while(x<=xlimits.first) { x+=binw/100; }
		while(x>=xlimits.second) { x-=binw/100; }
		auto lim = GetMinMaxLocal(ysys,xsys,x);
		//		ymin = invMassMin(ysys,xsys,x);
		//		ymax = invMassMax(ysys,xsys,x);

		xcoord[i]=x; ycoord[i]=lim.first;
		xcoord[num*2-i]=x; ycoord[num*2-i]=lim.second;
	}
	//adding last datapoint
	x = i*binw + xlimits.first;
	while(x<=xlimits.first) { x+=binw/100; }
	while(x>=xlimits.second) { x-=binw/100; }

	xcoord[i]=x; ycoord[i]=GetMinMaxLocal(ysys,xsys,x).first;
	return;
}

double DalitzKinematics::scatteringAngle(double s, double t,
		double M, double mSpec, double mSecond, double m)
{
	double u = getThirdVariableSq(s,t);
	double qSq = (s-(M+mSpec)*(M+mSpec))*(s-(M-mSpec)*(M-mSpec))/(4*s);
	double qPrimeSq = (s-(mSecond+m)*(mSecond+m))*(s-(mSecond-m)*(mSecond-m))/(4*s);
	double cosAngle = ( s*(t-u)+(M*M-mSpec*mSpec)*(mSecond*mSecond-m*m) )/(4*s*sqrt(qSq)*sqrt(qPrimeSq));

	return cosAngle;
}

double DalitzKinematics::helicityAngle(double M, double m, double m2, double mSpec,
		double invMassSqA, double invMassSqB)
{
	//Calculate energy and momentum of m1/m2 in the invMassSqA rest frame
	double eCms = ( invMassSqA + m*m - m2*m2 )/( 2*sqrt(invMassSqA) );
	double pCms = sqrt( eCms*eCms - m*m );
	//Calculate energy and momentum of mSpec in the invMassSqA rest frame
	double eSpecCms = ( M*M - mSpec*mSpec - invMassSqA )/( 2*sqrt(invMassSqA) );
	double pSpecCms = sqrt( eSpecCms*eSpecCms - mSpec*mSpec );
	double cosAngle = -( invMassSqB - m*m - mSpec*mSpec - 2*eCms*eSpecCms )/( 2*pCms*pSpecCms );

	if( cosAngle>1 || cosAngle<-1 ){
		throw BeyondPhsp("DalitzKinematics::helicityAngle() | "
				"scattering angle out of range! Datapoint beyond phsp? angle="
				+std::to_string((long double) cosAngle)
		+" M="+std::to_string((long double) M)
		+" m="+std::to_string((long double) m)
		+" m2="+std::to_string((long double) m2)
		+" mSpec="+std::to_string((long double) mSpec)
		+" mSqA="+std::to_string((long double) invMassSqA)
		+" mSqB="+std::to_string((long double) invMassSqB) );
	}

	return cosAngle;
}

double DalitzKinematics::helicityAngle(unsigned int sys, dataPoint& point)
{
	return helicityAngle(sys,point.getVal(0),point.getVal(1));
}

double DalitzKinematics::helicityAngle(unsigned int sys,
		double invMassSq23, double invMassSq13)
{
	if( sys > 2 )
		throw WrongVariableID("DalitzKinematics::helicityAngle() | "
				"Wrong variable ID! Invariant masses are number with 0,1 and 2!");

	double invMassSq12 = getThirdVariableSq(invMassSq23,invMassSq13);
	//	double invMsqSys, invMsqSecond;
	//	double m, mSpec, eCms, eSpecCms, pCms, pSpecCms;
	double cosAngle;
	switch(sys){
	case 2://angle versus particle 2
		//		m = m2; mSpec = m3; invMsqSys = invMassSq12; invMsqSecond = invMassSq23;
		//		eCms = eiCms(2,sys,invMsqSys);
		//		eSpecCms = eiCms(3,sys,invMsqSys);
		//		pCms = sqrt(eCms*eCms-m*m);
		//		pSpecCms = sqrt(eSpecCms*eSpecCms-mSpec*mSpec);
		cosAngle = helicityAngle(M,m2,m1,m3,invMassSq12,invMassSq23);
		break;
	case 1://angle versus particle 1
		//		m = m1; mSpec = m2; invMsqSys = invMassSq13; invMsqSecond = invMassSq12;
		//		eCms = eiCms(1,sys,invMsqSys);
		//		eSpecCms = eiCms(2,sys,invMsqSys);
		//		pCms = sqrt(eCms*eCms-m*m);
		//		pSpecCms = sqrt(eSpecCms*eSpecCms-mSpec*mSpec);
		cosAngle = helicityAngle(M,m1,m3,m2,invMassSq13,invMassSq12);
		break;
	case 0:
		//angle versus particle 2
		//		m = m2; mSpec = m1; invMsqSys = invMassSq23; invMsqSecond = invMassSq12;
		//		eCms = eiCms(2,sys,invMsqSys);
		//		eSpecCms = eiCms(1,sys,invMsqSys);
		//		pCms = sqrt(eCms*eCms-m*m);
		//		pSpecCms = sqrt(eSpecCms*eSpecCms-mSpec*mSpec);
		cosAngle = helicityAngle(M,m2,m3,m1,invMassSq23,invMassSq12);
		break;
	}
	//	cosAngle = -(invMsqSecond-m*m-mSpec*mSpec-2*eCms*eSpecCms)/(2*pCms*pSpecCms);

	return cosAngle;
}

//! returns 1 if point is within PHSP otherwise 0
double phspFunc(double* x, size_t dim, void* param)
{
	if(dim!=2) return 0;
	dataPoint point;
	try{
		Kinematics::instance()->FillDataPoint(0,1,x[1],x[0],point);
	} catch (BeyondPhsp& ex){
		return 0;
	}
	return 1.0;
};

double DalitzKinematics::getPhspVolume()
{
	if(!_DPareaCalculated) calcDParea();
	return _DParea;
}

void DalitzKinematics::calcDParea()
{
	size_t dim=2;
	double res=0.0, err=0.0;

	//set limits: we assume that x[0]=m13sq and x[1]=m23sq
	auto m13_limit = GetMinMax(1);
	auto m23_limit = GetMinMax(0);
	double xLimit_low[2] = {m13_limit.first,m23_limit.first};
	double xLimit_high[2] = {m13_limit.second,m23_limit.second};

	size_t calls = 2000000;
	gsl_rng_env_setup ();
	const gsl_rng_type *T = gsl_rng_default; //type of random generator
	gsl_rng *r = gsl_rng_alloc(T); //random generator

	gsl_monte_function F = {&phspFunc,dim, const_cast<DalitzKinematics*> (this)};

	gsl_monte_vegas_state *s = gsl_monte_vegas_alloc (dim);
	gsl_monte_vegas_integrate (&F, xLimit_low, xLimit_high, 2, calls, r,s,&res, &err);
	gsl_monte_vegas_free(s);
	BOOST_LOG_TRIVIAL(debug)<<"DPKinematics::calcDParea() Dalitz plot area (MC integration): "
			<<"("<<res<<"+-"<<err<<") GeV^4 relAcc [%]: "<<100*err/res;

	_DParea=res;
	_DPareaCalculated=1;
	return;
}

unsigned int DalitzKinematics::getSpin(unsigned int num)
{
	switch(num){
	case 0: return spinM;
	case 1: return spin1;
	case 2: return spin2;
	case 3: return spin3;
	}
	throw std::runtime_error("DPKinematics::getSpin(int) | "
			"Wrong particle requested!");
	return -999;
}
unsigned int DalitzKinematics::getSpin(std::string name)
{
	if(name==nameMother) return spinM;
	if(name==name1) return spin1;
	if(name==name2) return spin2;
	if(name==name3) return spin3;
	throw std::runtime_error("DPKinematics::getSpin(string) | "
			"Wrong particle "+name+" requested!");
}

double DalitzKinematics::getMass(unsigned int num)
{
	switch(num){
	case 0: return M;
	case 1: return m1;
	case 2: return m2;
	case 3: return m3;
	}
	throw std::runtime_error("DPKinematics::getMass(int) | "
			"Wrong particle requested!");
}
double DalitzKinematics::getMass(std::string name)
{
	if(name==nameMother) return M;
	if(name==name1) return m1;
	if(name==name2) return m2;
	if(name==name3) return m3;
	throw std::runtime_error("DPKinematics::getMass(string) | "
			"Wrong particle "+name+" requested!");
}

//! Calculates 3rd invariant mass from the other inv. masses.
double DalitzKinematics::getThirdVariableSq(double invmass1sq, double invmass2sq) const
{
	return (Msq+mSq1+mSq2+mSq3-invmass1sq-invmass2sq);
}

bool DalitzKinematics::isWithinPhsp(const dataPoint& point)
{
	double m23sq = point.getVal(0);
	double m13sq = point.getVal(1);
	double m12sq = point.getVal(2);

	if(m12sq < GetMinMaxLocal(2,0,m23sq).first || m12sq > GetMinMaxLocal(2,0,m23sq).second) return 0;
	if(m13sq < GetMinMaxLocal(1,0,m23sq).first || m13sq > GetMinMaxLocal(1,0,m23sq).second) return 0;
	if(m23sq < GetMinMaxLocal(0,1,m13sq).first || m23sq > GetMinMaxLocal(0,1,m13sq).second) return 0;
	if(m12sq < GetMinMaxLocal(2,1,m13sq).first || m12sq > GetMinMaxLocal(2,1,m13sq).second) return 0;
	if(m13sq < GetMinMaxLocal(1,2,m12sq).first || m13sq > GetMinMaxLocal(1,2,m12sq).second) return 0;
	if(m23sq < GetMinMaxLocal(0,2,m12sq).first || m23sq > GetMinMaxLocal(0,2,m12sq).second) return 0;
	return 1;

}
