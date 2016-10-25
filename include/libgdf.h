#include "datafile.h"

#ifndef LIBGDF_H
#define LIBGDF_H

namespace gdf
{
class Reader;
}

/**
 * @brief A class implementing the GDF v2 file type.
 *
 * All methods accessing the information stored in the file are thread-safe.
 */
class LibGDF : public DataFile
{
public:
	/**
	 * @brief
	 * @param filePath The file path of the data file without the extension.
	 */
	LibGDF(const std::string& filePath/*, bool uncalibrated = false*/);
	virtual ~LibGDF();

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
	virtual void readSignalFromFile(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readSignalFromFile(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}

protected:
	/**
	 * @brief
	 * @return
	 */
	virtual bool load() override;

private:
	double samplingFrequency;
	uint16_t numberOfChannels;
	uint64_t samplesRecorded;
	gdf::Reader* gdfReader;
	int readChunk;
	double* readChunkBuffer;

	template<typename T>
	void readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);
};

#endif // LIBGDF_H
