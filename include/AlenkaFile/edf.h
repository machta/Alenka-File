#include <AlenkaFile/datafile.h>

#ifndef ALENKAFILE_EDF_H
#define ALENKAFILE_EDF_H

class edf_hdr_struct;

namespace AlenkaFile
{

/**
 * @brief A class implementing the EDF+ and BDF+ types.
 *
 * There is a limit on the channel count (512) due to the limitations
 * of the EDFlib library.
 */
class EDF : public DataFile
{
public:
	/**
	 * @brief
	 * @param filePath The file path of the primary data file.
	 */
	EDF(const std::string& filePath);
	virtual ~EDF();

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
	int numberOfChannels;
	uint64_t samplesRecorded;
	edf_hdr_struct* edfhdr;
	int readChunk;
	double* readChunkBuffer;

	template<typename T>
	void readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);
};

} // namespace AlenkaFile

#endif // ALENKAFILE_EDF_H
