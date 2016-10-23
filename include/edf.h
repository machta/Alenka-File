#include "datafile.h"

#ifndef EDF_H
#define EDF_H

class edf_hdr_struct;

/**
 * @brief A class implementing the EDF+ and BDF+ types.
 *
 * There is a limit on the channel count (512) dueto the limitations
 * of theEDFlib library.
 *
 * It is assumed that all channels have the same sampling frequency and
 * the same length.
 */
class EDF : public DataFile
{
public:
	/**
	 * @brief
	 * @param filePath The file path of the data file without the extension.
	 */
	EDF(const std::string& filePath, const std::string& type = "edf");
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
	virtual void save() override;

protected:
	virtual void readSignalFromFile(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readSignalFromFile(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}

	/**
	 * @brief
	 * @return
	 */
	virtual bool load() override;

private:
	double samplingFrequency;
	uint16_t numberOfChannels;
	uint64_t samplesRecorded;
	edf_hdr_struct* edfhdr;
	int readChunk;
	double* readChunkBuffer;

	template<typename T>
	void readSignalFromFileFloatDouble(std::vector<T*> dataChannels, uint64_t firstSample, uint64_t lastSample);
};

#endif // EDF_H
