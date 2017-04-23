#ifndef ALENKAFILE_LIBGDF_H
#define ALENKAFILE_LIBGDF_H

#include <AlenkaFile/datafile.h>

namespace gdf
{
class Reader;
}

namespace AlenkaFile
{

/**
 * @brief A class implementing the GDF file type.
 */
class LibGDF : public DataFile
{
	double samplingFrequency;
	uint16_t numberOfChannels;
	uint64_t samplesRecorded;
	gdf::Reader* gdfReader;
	int readChunk;
	double* readChunkBuffer;

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
	virtual void save() override;
	virtual bool load() override;
	virtual void readChannels(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readChannels(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}

private:
	template<typename T>
	void readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);
};

} // namespace AlenkaFile

#endif // ALENKAFILE_LIBGDF_H
