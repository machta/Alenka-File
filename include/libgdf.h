#include "datafile.h"

#ifndef LIBGDF_H
#define LIBGDF_H

namespace gdf
{
class Reader;
}

/**
 * @brief A class implementing the GDF file type.
 */
class LibGDF : public DataFile
{
public:
	/**
	 * @brief
	 * @param filePath The file path of the primary data file.
	 */
	LibGDF(const std::string& filePath);
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
	virtual void save(pugi::xml_document* const infoFile) override;
	virtual bool load(pugi::xml_document* infoFile) override;
	virtual void readSignalFromFile(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readSignalFromFile(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}

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
