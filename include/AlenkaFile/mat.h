#ifndef ALENKAFILE_MAT_H
#define ALENKAFILE_MAT_H

#include <AlenkaFile/datafile.h>

#include <vector>

typedef struct _mat_t mat_t;
struct matvar_t;

namespace AlenkaFile
{

struct MATvars
{
	std::string
		data = "d",
		frequency = "fs",
		multipliers = "mults",
		date = "date",
		header = "header",
		label = "label",
		events = "events";

	int
		positionIndex = 0,
		durationIndex = 0;
};

class MAT : public DataFile
{
	MATvars vars;
	mat_t* file;
	double samplingFrequency;
	int numberOfChannels;
	uint64_t samplesRecorded;
	std::vector<char> tmpBuffer;
	std::vector<matvar_t*> data;
	std::vector<int> sizes;
	std::vector<double> multipliers;
	double days = daysUpTo1970;

public:
	MAT(const std::string& filePath, const MATvars& vars = MATvars());
	virtual ~MAT();

	virtual double getSamplingFrequency() const override
	{
		return samplingFrequency;
	}
	virtual unsigned int getChannelCount() const override
	{
		return numberOfChannels;
	}
	virtual uint64_t getSamplesRecorded() const override
	{
		return samplesRecorded;
	}
	virtual double getStartDate() const override
	{
		return days;
	}
	virtual void save() override;
	virtual bool load() override;
	virtual void readChannels(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readChannelsFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readChannels(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readChannelsFloatDouble(dataChannels, firstSample, lastSample);
	}

private:
	void readSamplingRate();
	void readData();
	void readMults();
	void readDate();
	std::vector<std::string> readLabels();
	void readEvents(std::vector<int>* eventPositions, std::vector<int>* eventDurations);

	template<typename T>
	void readChannelsFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);

	void fillDefaultMontage();
	bool readDataVar(const std::string& varName);
	void loadEvents();
};

} // namespace AlenkaFile

#endif // ALENKAFILE_MAT_H
