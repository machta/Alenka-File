#ifndef ALENKAFILE_MAT_H
#define ALENKAFILE_MAT_H

#include <AlenkaFile/datafile.h>

#include <vector>

typedef struct _mat_t mat_t;
struct matvar_t;

namespace AlenkaFile
{

class MAT : public DataFile
{
	const std::string dataVarName, frequencyVarName, multipliersVarName, dateVarName;
	mat_t* file;
	double samplingFrequency;
	int numberOfChannels;
	uint64_t samplesRecorded;
	std::vector<char> tmpBuffer;
	std::vector<matvar_t*> data;
	std::vector<int> sizes;
	std::vector<double> multipliers;
	double date = daysUpTo1970;

public:
	MAT(const std::string& filePath, const std::string& dataVarName = "data", const std::string& frequencyVarName = "Fs",
		const std::string& multipliersVarName = "mults", const std::string& dateVarName = "date");
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
	virtual double getStartDate() const override;
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
	template<typename T>
	void readChannelsFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);

	void fillDefaultMontage();
	bool readDataVar(const std::string& varName);
};

} // namespace AlenkaFile

#endif // ALENKAFILE_MAT_H
