#ifndef _SNOW_LAYER_H_
#define _SNOW_LAYER_H_

#include <string>
#include <vector>
#include <sstream>

#define MIN_DENSITY		0.04
#define MIN_MAXDENSITY	0.22
#define MAX_MAXDENSITY	0.55

#define MIN_PARTICLE	0.1
#define MAX_PARTICLE1	0.6
#define MIN_PARTICLE2	1
#define MAX_PARTICLE2	1.2
#define MIN_PARTICLE3	2
#define MAX_PARTICLE3	10

#define K_CONDUCTIVITY 5.13

#define SIGNED(x)	((x) < 0 ? -1 : 1)

class CSnow;

class CLayer
{
private:
	// all layers are owned by a CSnow
	CSnow *_snow;
	//index (number) of this layer
	CLayer* _top;
	CLayer* _bottom;
	//selection flag
	bool _selection;
	//time in hours
	double _time0;
	double _life_time;
	double _duration;
	double _density0;
	double _particle0;
	//height of snow layer in mm
	double _height0; //mm
			
	struct STEP
	{
		//double time0;
		double density; // g/sm^3
		double particle; // mm
		double height; // mm
		
		double weight() const { return density*height*0.1; } // g/sm^2

		STEP(double d, double p, double h) : density(d), particle(p), height(h) {}
	};

	std::vector<STEP> data;

	unsigned char Value(double ml, double m2, double h);
	double GetSnowWeight(double time);
	double GetMaxDensity(double weight);
	double GetMaxParticle(double dT, double particle);
	STEP GetStep(double time);
public:
	//std::vector<int> physicals;
	CLayer(CSnow* s, double density, double height, double time = 0, double particle = MIN_PARTICLE, double life_time = 0, double duration = 0);
	~CLayer();

	CSnow* Snow() const { return _snow; }

	void SetTop(CLayer* layer);
	void SetBottom(CLayer* layer);

	CLayer* GetTop() { return _top; }
	CLayer* GetBottom() { return _bottom; }

	void Reset();

	bool GetSelection() const { return _selection; }
	void SetSelection(bool selection) { _selection = selection; }

	double GetTime0(bool end = false) const { return _time0 + (end ? _duration : 0); } //hours
	void SetTime0(double time) { _time0 = time; }
	double GetLifeTime() const { return _life_time;}
	void SetLifetime(double time) { _life_time = time; }
	double GetDuration() const { return _duration;}
	void SetDuration(double duration) { _duration = duration; }
	bool ValidTime(double time) const { return !(time < _time0 || (_life_time && (time > _time0 + _life_time))); }

	double GetDensity0() const { return _density0;}
	void SetDensity0(double density) { _density0 = density; }
	double GetParticleSize0() const { return _particle0;}
	void SetParticleSize0(double size) { _particle0 = size; }
	double GetHeight0() const { return _height0;}
	void SetHeight0(double height) { _height0 = height; }

	double GetDensity(double time); //g/sm^3
	double GetDensity();
	double GetHeight(double time); //mm
	double GetHeight();
	double GetParticle(double time); //mm
	double GetParticle();
	//double GetWeight(double time) const { return (ValidTime(time)) ? data[0].density*_height/10 : 0; } //g/sm^2
	double GetWeight(double time) { return (ValidTime(time)) ? GetDensity(time)*GetHeight(time)/10 : 0; } //g/sm^2
	
	double GetTemperature(double time);
	double GetTemperature();
	double GetBottomTemperature(double time);
	double GetBottomTemperature();
	double GetTopTemperature(double time);
	double GetTopTemperature();
	
	unsigned int GetColor(double h, double l = 0.3, double s = 1);
	unsigned int GetDensityColor(double time);
	unsigned int GetDensityColor();
	unsigned int GetParticleColor(double time, bool selection = false);
	unsigned int GetParticleColor();
	unsigned int GetParticleSelectionColor();

	unsigned int HLStoRGB(double h, double l, double s);

	std::wstring ToString(int parameter = ALL, int precision = -1);
	std::wstring ToString(double time, int parameter = ALL, int precision = -1);

	enum PARAMETERS { ALL = 0, DENSITY, HEIGHT, PARTICLE, TEMPERATURE };
};

#endif //_SNOW_LAYER_H_
