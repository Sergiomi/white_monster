#include "stdafx.h"
#include <stdlib.h>
#include <fstream>
#include "CSnowLayer.h"
#include "CSnow.h"

#define VALID_SOLVE(x, time) { \
	static double last_time; \
	if (last_time != (time) || time == 0) last_time = time; \
	else return x; } 

std::vector<CSnow*> CSnow::list;
int CSnow::_current = -1;

CSnow::CSnow(std::wstring name)
	: _name(name), _visible(1), _currentLayer(-1), _currentTime(1), _hResulition(4), _baseT(-1)
{
	// hide all other models
	for(unsigned int i = 0; i < list.size(); i++)
		list[i]->setVisibility(0);

	// push new one into the list
	list.push_back(this);

	addTLog(0, _baseT);
	setAsCurrent();
}

CSnow::~CSnow()
{
	std::vector<CSnow*>::iterator it = std::find(list.begin(), list.end(), this);
	if(it != list.end()) list.erase(it);
	destroy();
}

CSnow *CSnow::current(int index)
{
	if(list.empty()){
		//Msg::Info("No current model available: creating one");
		new CSnow();
	}
	if(index >= 0 && index <= list.size()) _current = index;
	if(_current < 0 || _current >= (int)list.size()) return list.back();
	return list[_current];
}

int CSnow::setCurrent(CSnow *s)
{
	for (unsigned int i = 0; i < list.size(); i++){
		if (list[i] == s){
			_current = i;
			break;
		}
	}
	return _current;
}

CSnow *CSnow::findByName(std::wstring name)
{
	// return last mesh with given name
	for(int i = list.size() - 1; i >= 0; i--)
		if(list[i]->getName() == name) return list[i];
	return 0;
}

void CSnow::destroy()
{
	for(unsigned int i = 0; i < snowLayers.size(); i++)
		delete snowLayers[i];
	snowLayers.clear();

	_currentLayer = -1;
}


bool CSnow::empty() const
{
	return snowLayers.empty();
}

bool CSnow::remove(CLayer* l)
{
	l_iter it = std::find(snowLayers.begin(), snowLayers.end(), l);
	if(it != snowLayers.end())
	{
		if(l == snowLayers[_currentLayer])
			_currentLayer = -1;

		delete *it;
		snowLayers.erase(it);

		resetSnow();

		return true;
	}
	return false;
}

void CSnow::add(double density, double height, double time, double particle, double life_time,  double duration)
{
	resetSnow();
	
	CLayer* new_layer = new CLayer(this, density, height, time, particle, life_time, duration);
	if (snowLayers.empty() || snowLayers[snowLayers.size()-1]->GetTime0() <= time) {
		if (!snowLayers.empty())
			new_layer->SetBottom(snowLayers[snowLayers.size()-1]);
		snowLayers.push_back(new_layer);
		return;
	}
	
	for(unsigned int i = snowLayers.size() - 1; i >= 0 ; i--) {
		if (snowLayers[i]->GetTime0() <= time) {
			new_layer->SetBottom(snowLayers[i]);
			new_layer->SetTop(snowLayers[i + 1]);
			snowLayers.insert(snowLayers.begin() + i + 1, new_layer);
			return;
		}
		//snowLayers[i]->SetIndex(i + 1);
	}
	new_layer->SetTop(snowLayers[0]);
	snowLayers.insert(snowLayers.begin(), new_layer);
}

void CSnow::reset()
{
	T_Log.clear();
	baseT_Log.clear();

	resetSnow();
}

void CSnow::resetSnow()
{
	baseT_Log.clear();

	for(unsigned int i = 0; i < snowLayers.size(); i++) {
		snowLayers[i]->Reset();
	}
}

bool CSnow::remove(unsigned int idx) 
{
	if(idx >= 0 && idx < snowLayers.size())
	{
		if(idx == _currentLayer)
		_currentLayer = -1;

		delete snowLayers[idx];
		snowLayers.erase(snowLayers.begin() + idx);
		
		resetSnow();
		return true;
	}
	return false;
}

void CSnow::setSelection(unsigned int val)
{
	if (val >= snowLayers.size())
		return;

	for(unsigned int i = 0; i < snowLayers.size(); i++){
		snowLayers[i]->SetSelection(i==val);
	}
	_currentLayer = val;
}

double CSnow::getSnowHeight(double time)
{
	static double H;
	VALID_SOLVE(H, time);
	
	double height = 0;
	for(unsigned int i = 0; i < snowLayers.size(); i++)
		height += snowLayers[i]->GetHeight(time);
	
	H = height;
	return H;
}
double CSnow::getSnowHeight(double time1, double time2)
{
	double height;
	double time0;

	if (time1 == time2)
		return getSnowHeight(time1);

	if (time2 < time1) {
		double t = time1;
		time1 = time2;
		time2 = t;
	}

	height = max(getSnowHeight(time1), getSnowHeight(time2));
	for(unsigned int i = 0; i < snowLayers.size(); i++) {
		time0 = snowLayers[i]->GetTime0(true);
		if (time0 > time1 && time0 < time2) {
			height = max(getSnowHeight(time0), height);
		}
	}

	return height;
}

double CSnow::getSnowDensity(double time)
{
	static double D;

	VALID_SOLVE(D, time);

	double density = 0;
	for(unsigned int i = 0; i < snowLayers.size(); i++)
		density += snowLayers[i]->GetDensity(time);

	D = density;
	return D;
}

int CSnow::addTLog(double time, double T)
{
	if (time < 0)
		return T_Log.size();

	resetSnow();

	if (T_Log.empty() || time >= T_Log.rbegin()->first) {
		T_Log.push_back(std::pair<double, double>(time, T));
		return T_Log.size();
	}

	for(unsigned int i = T_Log.size() - 1; i >= 0 ; i--) {
		if (T_Log[i].first <= time) {
			T_Log.insert(T_Log.begin() + i + 1, std::pair<double, double>(time, T));
			return T_Log.size();
		}
	}
	T_Log.insert(T_Log.begin(), std::pair<double, double>(time, T));
	return T_Log.size();
}
int CSnow::removeTLog(int i)
{
	if (i >= T_Log.size() || i < 0)
		return T_Log.size();
	
		resetSnow();
		T_Log.erase(T_Log.begin() + i);
	
	return T_Log.size();
}
void CSnow::editTLog(size_t i, double time, double T)
{ 
	if (i >= T_Log.size() || i < 0)
		return;

	resetSnow();
	T_Log[i].first = time;
	T_Log[i].second = T;
}

double CSnow::getTemperature(double time)
{
	static double T;	
	VALID_SOLVE(T, time);

	if (T_Log.empty()) {
		T = _baseT;
		return T;
	}

	for (int i = 0; i < T_Log.size(); i++) {
		if (T_Log[i].first == time) {
			T = T_Log[i].second;
			return T;
		}

		if (T_Log[i].first > time) {
			if (i > 0) {
				T = T_Log[i-1].second + (time - T_Log[i-1].first)*(T_Log[i].second - T_Log[i-1].second)/(T_Log[i].first - T_Log[i-1].first);
				return T;
			}
			T = T_Log.begin()->second;
			return T;
		}
	}
	T = T_Log.rbegin()->second;
	return T;
}

std::pair<double, double> CSnow::getMinMaxTemperature(double time1, double time2)
{
	double minT;
	double maxT;

	if (T_Log.empty())
		return std::pair<double, double>(_baseT, _baseT);

	minT = maxT = 0;

	for (int i = 0; i < T_Log.size(); i++) {
		if (T_Log[i].first < time1)
			continue;
		minT = min(T_Log[i].second, minT);
		maxT = max(T_Log[i].second, maxT);
		if (T_Log[i].first > time2)
			break;
	}

	return std::pair<double,double>(minT, maxT);
}

double CSnow::getBaseTemperature(double time)
{
	if (time <= 0)
		return _baseT;

	if (baseT_Log.empty())
		baseT_Log.push_back(_baseT);

	int discretTime = (int)(time/getTimeStep());
	if(discretTime < baseT_Log.size())
		return baseT_Log[discretTime];

	const double dtime = 24*3;
	
	for (int i = baseT_Log.size(); i <= discretTime; i++) {
		double t = i*getTimeStep();
		int dt = (int)(i - dtime/getTimeStep());
		if (dt < 0) dt = 0;

		double T = getTemperature(t);
		double h = getSnowHeight(t);
		if (h == 0) {
			baseT_Log.push_back(T);
			continue;
		}

		double bT = baseT_Log[dt];
		double bT1 =  T*exp(-K_CONDUCTIVITY*h*0.001) - 1;
		double baseT = (bT1 + bT)/2;
		//double baseT =  (T*exp(-K_CONDUCTIVITY*h*0.001) + baseT_Log[dt])/2;
		if (baseT > 0) baseT = 0;
		baseT_Log.push_back(baseT);
	}

	return baseT_Log[discretTime];
	/*static double baseT = 0;
	VALID_SOLVE(baseT, time);

	if (time <= 0) {
		baseT = _baseT;
		return baseT;
	}
	
	double T = getTemperature(time);

	double height = getSnowHeight(time);
	if (height == 0) {
		baseT = T;
		return baseT;
	}

	baseT =  T*exp(-K_CONDUCTIVITY*height*0.001);
	if (baseT > 0) baseT = 0;
	return baseT;*/
}

double CSnow::getTGradient(double time)
{
	static double dT = 0;
	VALID_SOLVE(dT, time);

	double height = getSnowHeight(time);
	if (height == 0) {
		dT = 0;
		return dT;
	}

	double T = getTemperature(time);
	double baseT = getBaseTemperature(time);
	if (baseT > 0) baseT = 0;
	dT = (baseT - T)/height*1000;
	//double dT =  (_baseT - T)/height*1000;

	//if (dT < 0)
	//	return 0;
	return dT;
}

std::wstring CSnow::ToString(double time, int parameter, int precision)
{ 
	std::wostringstream sstream;

	if(precision != -1)
		sstream.precision(precision);
	
	switch(parameter)
	{
	case ALL: 
		//sstream.precision(4); 
		sstream<<"Time = "<<time<<"\r\n";
		sstream<<"Snow Height = "<<getSnowHeight(time)<<"\r\n";
		sstream<<"T = "<<getTemperature(time)<<"\r\n";
		sstream<<"base T = "<<getBaseTemperature(time)<<"\r\n";
		sstream<<"gradientT = "<<getTGradient(time)<<"\r\n";
		sstream<<"\r\n"<<getLayersNum()<<" Layers of Snow\r\n";
		if(_currentLayer < 0)
			break;
		sstream<<"\r\nLayer number "<<_currentLayer + 1<<" info:\r\n";
		sstream<< snowLayers[_currentLayer]->ToString();
		//t0 = snowLayers[_currentLayer]->GetTime0();
		//sstream<<"t0 = "<<t0<<"\r\n";
		//sstream<<"Density(t0) = "<<snowLayers[_currentLayer]->GetDensity(t0)<<"\r\n";
		//sstream<<"Height(t0) = "<<snowLayers[_currentLayer]->GetHeight(t0)<<"\r\n\r\n";
		//sstream<<"Density = "<<snowLayers[_currentLayer]->GetDensity()<<"\r\n";
		//sstream<<"Height = "<<snowLayers[_currentLayer]->GetHeight()<<"\r\n";
		break;
	case TIME: sstream<<time; break;
	case HEIGHT: sstream<<getSnowHeight(time); break;
	case TEMPERATURE: sstream<<getTemperature(time); break;
	case BASE_TEMPERATURE: sstream<<getBaseTemperature(time); break;
	case GRADIENT_TEMPERATURE: sstream<<getTGradient(time); break;
	case LAYERS_COUNT: sstream<<getLayersNum(); break;
	}
	return sstream.str(); 
}

int CSnow::readFile(const std::wstring &name)
{
	if (name.empty())
		return 0;

	FILE *fp = _wfopen(name.c_str(), _T("rb"));
	if(!fp){
		return 0;
	}

	setFileName(name);

	int ch;
	std::string str;
	int type = 0;
	double dInput[6];
	int count;
	
	while(1) {
		str.clear();
		do {
			if(!(ch = fgetc(fp)) || feof(fp))
				break;
			str += ch;
		} while(ch != '\n');

		if(!str.empty())
		{
			if(str[0] == '#')
				continue;

			if(str[0] == '$')
				type = 0;

			if(!type)
			{
				if(str.find("$LAYERS") != std::string::npos) {
					type = 1;
					this->destroy();
				}
				else if(str.find("$TEMPERATURES") != std::string::npos) {
					type = 2;
					this->reset();
				}
			}
			else
			{
				switch(type)
				{
				case 1: 
					count = sscanf_s(str.c_str(), "%lf %lf %lf %lf %lf", &dInput[0], &dInput[1], &dInput[2], &dInput[3], &dInput[4], &dInput[5]); 
					switch (count) {
					case 3:
						add(dInput[0], dInput[1], dInput[2]);
						break;
					case 4:
						add(dInput[0], dInput[1], dInput[2], dInput[3]); 
						break;
					case 5:
						add(dInput[0], dInput[1], dInput[2], dInput[3], dInput[4]);
						break;
					case 6:
						add(dInput[0], dInput[1], dInput[2], dInput[3], dInput[4], dInput[5]);
						break;
					}
					break;
				case 2:
					if (sscanf_s(str.c_str(), "%lf %lf", &dInput[0], &dInput[1]) == 2)
					{
						if (dInput[0] == 0)
							_baseT = dInput[1];

						addTLog(dInput[0], dInput[1]);
					}
					break;
				}
				continue;
			}
		}

		if(feof(fp))
			break;
	}
	
	fclose(fp);
	return 1;
}

int CSnow::writeFile()
{
	if (getFileName().empty())
		return 0;

	std::ofstream out;
	out.open(getFileName());
	if (!out.is_open())
		return 0;

	out << "$LAYERS" << std::endl;
	out << "#DENSITY(g/sm^3)\tHEIGHT(mm)\tTIME0(hour)\tPARTICLE_SIZE(mm)\tLIFE_TIME(hour)\tDURATION(hour)" << std::endl;
	
	for (size_t i = 0; i < getLayersNum(); i++)
	{
		CLayer* layer = getLayer(i);
		out << layer->GetDensity0() << "\t";
		out << layer->GetHeight0() << "\t";
		out << layer->GetTime0() << "\t";
		out << layer->GetParticleSize0() << "\t";
		out << layer->GetLifeTime() << "\t";
		out << layer->GetDuration() << std::endl;
	}

	out << std::endl;
	out << "$TEMPERATURES" << std::endl;
	out << "#TIME(hour)\tTEMPERATURE(C)" << std::endl;

	for (size_t i = 0; i < getTLogSize(); i++)
		out << getTLog(i).first << "\t" << getTLog(i).second << std::endl;
		
	out.close();
	return 1;
}
std::wstring ToString(double d, int precision)
{
	std::wostringstream sstream;

	if(precision != -1)
		sstream.precision(precision);

	sstream << d;

	return sstream.str();
}

void CSnow::setFileName(std::wstring name)
{
	_fileName = name; 
	int n = _fileName.rfind(_T("\\"), _fileName.size() - 1);
	if (n < 0)
		n = 0;

	_name = _fileName.substr(n + 1, _fileName.size() - n - 1);
}