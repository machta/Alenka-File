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
	mat_t* file;
	double samplingFrequency;
	int numberOfChannels;
	uint64_t samplesRecorded;
	std::vector<char> tmpBuffer;
	std::vector<matvar_t*> data;
	std::vector<int> sizes;

public:
	MAT(const std::string& filePath);
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
	virtual time_t getStartDate(int timeZone = 0) const override;
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
};

} // namespace AlenkaFile

#endif // ALENKAFILE_MAT_H
