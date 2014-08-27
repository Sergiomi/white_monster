#ifndef _CSNOW_H_
#define _CSNOW_H_

#include <algorithm>
#include <vector>
#include <map>
#include <string>

#include "CSnowLayer.h"

class CSnowFactory;

class CSnow
{
 protected:
	// the name of the model
	std::wstring _name;
	std::wstring _fileName;

	// the visibility flag
	bool _visible;

	// index of the current model (in the static list of all loaded
	// models)
	static int _current;

	std::vector<CLayer*> snowLayers;
	int _currentLayer;
	double _currentTime;
	
	double _baseT;
	std::vector<std::pair<double, double>> T_Log; //first - time, second - T
	std::vector<double> baseT_Log;

	int _hResulition;
	
 public:
	CSnow(std::wstring name = _T("Unknown"));
	virtual ~CSnow();

	// the static list of all loaded models
	static std::vector<CSnow*> list;

	// return the current model, and sets the current model index if
	// index >= 0
	static CSnow *current(int index=-1);

	// sets a model to current
	static int setCurrent(CSnow *s);
	int setAsCurrent(){ return setCurrent(this); }

	// find a model by name
	static CSnow *findByName(std::wstring name);

	// delete everything in a GModel
	void destroy();

	// get/set the model name
	void setName(std::wstring name){ _name = name; }
	std::wstring getName(){ return _name; }

	void setFileName(std::wstring name);
	std::wstring getFileName(){ return _fileName; }

	// get/set the visibility flag
	bool getVisibility(){ return _visible; }
	void setVisibility(bool val){ _visible = val; }

	double getTime() const { return _currentTime; }
	void setTime(double time){ _currentTime = time; }

	//double getBaseTemperature() const { return _baseT; }
	void setBaseTemperature(double time, double T){ _baseT = T; }

	// get the number of entities in this model
	int getNumLayers() const { return snowLayers.size(); }

	double getTimeStep() const { return 1/(double)_hResulition; }
	// quickly check if the model is empty (i.e., if it contains no
	// entities)
	bool empty() const;

	typedef std::vector<CLayer*>::iterator l_iter;

	// add/remove an entity in the model
	void add(CLayer* l) { snowLayers.push_back(l); }
	bool remove(CLayer* l);
	void add(double density, double height, double time, double particle = MIN_PARTICLE, double life_time = 0, double duration = 0);
	bool remove(unsigned int idx);
	void reset();
	void resetSnow();
	
	CLayer* getLayer(unsigned int idx) { return (idx >= 0 && idx < snowLayers.size()) ? snowLayers[idx] : NULL; }
	CLayer* getLayer() { return getLayer(getSelection()); }
	int getLayersNum() { return snowLayers.size(); }
	int getSelection() { return _currentLayer; };

	// set the selection on the Layer
	void setSelection(unsigned int val);

	double getSnowHeight(double time);
	double getSnowHeight() { return getSnowHeight(_currentTime); }
	double getSnowHeight(double time1, double time2);

	double getSnowDensity(double time);
	double getSnowDensity() { return getSnowDensity(_currentTime); }

	double getTemperature(double time);
	double getTemperature() { return getTemperature(_currentTime); };
	double getTGradient(double time);
	double getTGradient() { return getTGradient(_currentTime); }
	double getBaseTemperature(double time);
	double getBaseTemperature() { return getBaseTemperature(_currentTime); }
	std::pair<double, double> getMinMaxTemperature(double time1, double time2);


	typedef std::vector<std::pair<double, double>>::iterator t_iter;
	typedef std::vector<std::pair<double, double>>::reverse_iterator t_riter;

	int getTLogSize() const { return T_Log.size(); }
	int addTLog(double time, double T);
	int removeTLog(int i);
	void editTLog(size_t i, double time, double T);
	std::pair<double, double> getTLog(int idx) const { return T_Log[idx]; }

	std::wstring ToString(double time, int parameter = ALL, int precision = -1);
	std::wstring ToString(int parameter = ALL, int precision = -1) { return ToString(_currentTime, parameter, precision); }
	enum PARAMETERS { ALL = 0, TIME, HEIGHT, TEMPERATURE, BASE_TEMPERATURE, GRADIENT_TEMPERATURE, LAYERS_COUNT};

	int readFile(const std::wstring &name);
	int readFile() { readFile(getFileName()); }
	int writeFile();
};

std::wstring ToString(double d, int precision = -1);
#endif
