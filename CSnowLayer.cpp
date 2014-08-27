#include "stdafx.h"
#include <math.h>
#include "CSnow.h"
#include "CSnowLayer.h"

double NormalDistribution(double x, double mean, double variance)
{
	return exp(-(x - mean)*(x - mean)/(2*variance*variance));
}

double FuzzyRange(double x, double left_mean, double left_variance, double middle_mean, double middle_variance, double right_mean, double right_variance)
{
	if (x < left_mean)
		return NormalDistribution(x, left_mean, left_variance)*NormalDistribution(left_mean, middle_mean, middle_variance);
	if (x < right_mean)
		return NormalDistribution(x, middle_mean, middle_variance);
	return NormalDistribution(x, right_mean, right_variance)*NormalDistribution(right_mean, middle_mean, middle_variance);
}

double FuzzyRange(double x, double left_mean, double left_variance, double right_mean, double right_variance)
{
	if (x < left_mean)
		return NormalDistribution(x, left_mean, left_variance);
	if (x <= right_mean)
		return 1;
	return NormalDistribution(x, right_mean, right_variance);
}

double FuzzyRange(double x, double mean, double variance) { return NormalDistribution(x, mean, variance); }

CLayer::CLayer(CSnow* s, double density, double height, double time, double particle, double life_time, double duration)
	: _snow(s), _selection(0), _density0(density), _height0(height), _time0(time), _particle0(particle), _duration(duration), _life_time(life_time)
{
	if (density < MIN_DENSITY) density = MIN_DENSITY;
	//else if (density > MAX_MAXDENSITY) density = MAX_MAXDENSITY;

	data.push_back(STEP(density, particle, height));

	_top = NULL;
	_bottom = NULL;
}

CLayer::~CLayer()
{
	data.clear();

	if (_bottom)
		_bottom->SetTop(_top);
	else if(_top)
		_top->SetBottom(_bottom);
}

void CLayer::Reset()
{
	data.clear();
	data.push_back(STEP(_density0, _particle0, _height0));
}

CLayer::STEP CLayer::GetStep(double time)
{
	if(!ValidTime(time))
		return STEP(0,0,0);

	int discretTime = (int)((time - _time0)/_snow->getTimeStep());
	if(discretTime < data.size())
		return data[discretTime];
	
	int i = data.size();
	for (; i <= discretTime; i++)
	{
		double p = data[i-1].particle;
		double d = data[i-1].density;
		double h = data[i-1].height;
		double t = (i-1)*_snow->getTimeStep() + _time0;

		if(h == 0)
			return STEP(0,0,0);
		
		double T = GetTemperature(t);
		double topT = GetTopTemperature(t);
		double dT = _snow->getTGradient(t);
		double weight = GetSnowWeight(t);

		if (topT < 0) {
			if (d >= MAX_MAXDENSITY) {
				data.push_back(STEP(d, p, h));
				continue;
			}

			double CdT1 = 0.0005*dT;//0.000015*pow(dT, 2)*(dT < 0 ? -1 : 1);
			double CdT2 = 0.0001*dT;
			
			double CdD = exp(-(d - 0.1));//*5*exp(-(p - MIN_PARTICLE)*2));
			//доделать зависимость
			double CdDP = exp(-((d/MAX_MAXDENSITY)/(p/MAX_PARTICLE2)*2*(1 - exp(-d))));
			//убрать скачек плотности
			//double CdP = (p > 0.8 /*&& d > MIN_MAXDENSITY*/) ? (exp(-(p - 0.8)*4)) : (1);
			double CdP = FuzzyRange(p, 0, 1, 0, 1.2, 1, 0.4);
			double CdLambda = exp(0.087*T);
			
			double pSmall1 = CdDP*(0.25 - p)*(1 - exp(-(_snow->getTimeStep())*0.05/(abs(T) < 1 ? 1 : abs(T))))*0.5;
			double pSmall2 = CdDP*((MAX_PARTICLE1 - p)*(1 - exp(-(_snow->getTimeStep())*weight*0.006)));
			pSmall1 = (pSmall1 < 0) ? 0 : pSmall1;
			pSmall2 = (pSmall2 < 0) ? 0 : pSmall2;
			//когда частицы большие, они растут в кристаллы при наличии градиента
			double pBigg1 = _snow->getTimeStep()*CdT1*CdD*FuzzyRange(p, 1.1, 0.3, MAX_PARTICLE2, 0.2);//FuzzyRange(p, 1, 0.25, MAX_PARTICLE2, 0.3);
			double pBigg2 = _snow->getTimeStep()*CdT2*CdD*(p > 1.2 ? (1 - FuzzyRange(p, MAX_PARTICLE2, 0.4, MAX_PARTICLE2, 0.2)) : 0);//FuzzyRange(p, MIN_PARTICLE3, 0.3, MAX_PARTICLE3, 2);
			double P = p + CdLambda*(pSmall1 + pSmall2 + pBigg1 + pBigg2);

			//double D = (d - GetMaxDensity(weight))*exp(-(_snow->getTimeStep())*weight*CdP*0.006*0.1) + GetMaxDensity(weight);
			double dSmall = (0.16 - d)*(1 - exp(-(_snow->getTimeStep())*0.05/(abs(T) < 1 ? 1 : abs(T))))*0.5;
			dSmall = (dSmall < 0) ? 0 : dSmall;
			double dBigg = CdP*(GetMaxDensity(weight) - d)*(1 - exp(-(_snow->getTimeStep())*weight*0.006*0.1));
			double D = d + dSmall + dBigg;
			//if (p < MAX_PARTICLE1) 
			//	D = max(D, P*0.24 + 0.076);
			if (D < d) D = d;
			data.push_back(STEP(D, P, h*d/D));
		} else {
			double D = d;
			double H = h;
			if (d < MAX_MAXDENSITY) {
				D = d + 0.05*0.05*_snow->getTimeStep()*(topT + 1);
				//if (D > MAX_MAXDENSITY) D = MAX_MAXDENSITY;
				H = h*d/D;
			} else if (_top == NULL || _top->GetHeight(t) == 0) {
				H = h - 10*d*0.05*_snow->getTimeStep()*(topT + 1);
				if (H < 0) H = 0;
			}
			data.push_back(STEP(D, p, H));
		}
	}

	return data[discretTime];
}

double CLayer::GetDensity(double time) { return GetStep(time).density;	}
double CLayer::GetDensity() { return GetDensity(_snow->getTime()); }
double CLayer::GetParticle(double time) { return GetStep(time).particle; }
double CLayer::GetParticle() { return GetParticle(_snow->getTime()); }

double CLayer::GetHeight(double time)
{
	//if(time == _time0)
	//	return _height;

	if(!ValidTime(time))
		return 0;

	if(time < _time0 + _duration) {
		return (time - _time0)*_height0/_duration*_density0/GetDensity(time);
	}

	return GetStep(time).height;
	//return _height0*data[0].density/GetDensity(time);
}

double CLayer::GetHeight() { return GetHeight(_snow->getTime()); }

double CLayer::GetTemperature(double time)
{
	if(!ValidTime(time))
		return 0;

	return (GetTopTemperature(time) + GetBottomTemperature(time))/2;
}

double CLayer::GetTemperature() { return GetTemperature(_snow->getTime()); }

double CLayer::GetBottomTemperature(double time)
{
	//return  GetTopTemperature(time)*exp(-K_CONDUCTIVITY*GetHeight(time)*0.001);
	double h = 0;
	double T;

	if (GetHeight(time) == 0)
		return GetTopTemperature(time);

	CLayer* bottom = _bottom;
	while(bottom) {
		h += bottom->GetHeight(time);
		bottom = bottom->GetBottom();
	}
	
	T = _snow->getBaseTemperature(time) - _snow->getTGradient(time)*h*0.001;
	return T > 0 ? 0 : T;

}

double CLayer::GetBottomTemperature() { return GetBottomTemperature(_snow->getTime()); };

double CLayer::GetTopTemperature(double time)
{
	if (_top) {
		return _top->GetBottomTemperature(time);
	}

	return _snow->getTemperature(time);
}

double CLayer::GetTopTemperature() { return GetTopTemperature(_snow->getTime()); };

unsigned int CLayer::GetColor(double h, double l, double s)
{
	return HLStoRGB(360*h, 0.65 - 0.5*l, 1);
}

unsigned int CLayer::GetDensityColor(double time)
{
	if(!ValidTime(time))
		return 0;

	double density = GetDensity(time);
	density = 0.9*(density - MIN_DENSITY)/(MAX_MAXDENSITY - MIN_DENSITY);
	if (density < 0) density = 0;
	if (density > 1) density = 1;
	return HLStoRGB(30, 1 - density, 0);
	//return //GetColor((density - MIN_DENSITY)/(MAX_MAXDENSITY - MIN_DENSITY));
}
unsigned int CLayer::GetDensityColor() { return GetDensityColor(_snow->getTime()); }
unsigned int CLayer::GetParticleColor(double time, bool selection)
{
	if(!ValidTime(time))
		return 0;

	double particle = GetParticle(time);
	double density = GetDensity(time);
	const double RED_PARTICLE = MIN_PARTICLE;
	const double YELLOW_PARTICLE = 0.25;
	const double GREEN_PARTICLE = 0.6;
	const double BLUE_PARTICLE = 2.2;
	const double VIOLET_PARTICLE = 5;

	double light = (density - MIN_DENSITY)/(MAX_MAXDENSITY - MIN_DENSITY);
	if (light > 1.1)light = 1.1;
	if (selection)light += 0.3;
	double hue;
	if (particle <= YELLOW_PARTICLE)
		hue = (particle - RED_PARTICLE)/(YELLOW_PARTICLE - RED_PARTICLE)/6;
	else if (particle <= GREEN_PARTICLE)
		hue = 1.0/6.0 + (particle - YELLOW_PARTICLE)/(GREEN_PARTICLE - YELLOW_PARTICLE)/6;
	else if (particle <= BLUE_PARTICLE)
		hue = 1.0/3.0 + (particle - GREEN_PARTICLE)/(BLUE_PARTICLE - GREEN_PARTICLE)/3;
	else { 
		if (particle > VIOLET_PARTICLE)
			particle = VIOLET_PARTICLE;
		hue = 2.0/3.0 + (particle - BLUE_PARTICLE)/(VIOLET_PARTICLE - BLUE_PARTICLE)/8;
	}

	if (hue < 0) hue = 0;
	return GetColor(hue, light);
	//double p = (particle - MIN_PARTICLE)/(MAX_MAXPARTICLE - MIN_PARTICLE);
	//double l = log(p*70 + 1)/log((double)70 + 1);
	//return GetColor(l, (density - MIN_DENSITY)/(MAX_MAXDENSITY - MIN_DENSITY));
}
unsigned int CLayer::GetParticleColor() { return GetParticleColor(_snow->getTime()); }
unsigned int CLayer::GetParticleSelectionColor() { return GetParticleColor(_snow->getTime(), true); }

unsigned char CLayer::Value(double m1, double m2, double h)
{
	if (h >= 360)
		h -= 360;
	else if (h < 0)
		h += 360;
	
	if (h < 60)
		m1 = m1 + (m2 - m1) * h / 60;
	else if (h < 180)
		m1 = m2;
	else if (h < 240)
		m1 = m1 + (m2 - m1) * (240 - h) / 60;
	
	return (unsigned char)(m1 * 255);
}

unsigned int CLayer::HLStoRGB(double h, double l, double s)
{
	if (s == 0)
		return RGB((unsigned char)(l*255), (unsigned char)(l*255), (unsigned char)(l*255));
	
	double m1, m2;

	m2 = l + ((l <= 0.5) ? (l) : (1 - l))*s;
	m1 = 2*l - m2;
	
	return RGB(Value(m1, m2, h + 120), Value(m1, m2, h), Value(m1, m2, h - 120));
}

std::wstring CLayer::ToString(double time, int parameter, int precision)
{ 
	std::wostringstream sstream; 
	
	if(precision != -1)
		sstream.precision(precision);

	switch(parameter)
	{
	case ALL: 
		sstream<<"t0 = "<<_time0;
		sstream<<"\r\nDensity = "<<GetDensity(time);
		sstream<<"\r\nHeight = "<<GetHeight(time);
		sstream<<"\r\nP Size = "<<GetParticle(time);
		sstream<<"\r\nT = "<<GetTemperature();
		sstream<<"\r\n\r\nDensity(t0) = "<<_density0;
		sstream<<"\r\nHeight(t0) = "<<_height0;
		sstream<<"\r\nP Size(t0) = "<<_particle0;
		break;
	case DENSITY: sstream<<GetDensity(time); break;
	case HEIGHT: sstream<<GetHeight(time); break;
	case PARTICLE: sstream<<GetParticle(time); break;
	case TEMPERATURE: sstream<<GetTemperature(time); break;
	}
	return sstream.str(); 
}

std::wstring CLayer::ToString(int parameter, int precision) { return ToString(_snow->getTime(), parameter, precision); }

double CLayer::GetSnowWeight(double time)
{
	double weight = GetWeight(time);

	CLayer* top = _top;
	while(top)
	{
		weight += top->GetWeight(time);
		top = top->GetTop();
	}
	
	return weight;
}

double CLayer::GetMaxDensity(double weight)
{
	return (MIN_MAXDENSITY - MAX_MAXDENSITY)*exp(-weight*0.04) + MAX_MAXDENSITY;
}

double CLayer::GetMaxParticle(double dT, double particle) 
{
	return max(particle, (MAX_PARTICLE1 - MAX_PARTICLE3)*exp(-dT*0.05) + MAX_PARTICLE3);
}

void CLayer::SetTop(CLayer* layer) 
{ 
	_top = layer; 
	if (layer && layer->GetBottom() != this) layer->SetBottom(this); 
}

void CLayer::SetBottom(CLayer* layer) 
{ 
	_bottom = layer;  
	if (layer && layer->GetTop() != this) layer->SetTop(this); 
}