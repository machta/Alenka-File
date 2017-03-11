#ifndef ALENKAFILE_GDF2_H
#define ALENKAFILE_GDF2_H

#include <AlenkaFile/datafile.h>

#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>

namespace AlenkaFile
{

/**
 * @brief A class implementing the GDF v2.51 file type.
 *
 * This is my own implementation that doesn't depend on anything but the standard library.
 */
class GDF2 : public DataFile
{
public:
	/**
	 * @brief GDF2 constructor.
	 * @param filePath The file path of the primary data file.
	 */
	GDF2(const std::string& filePath, bool uncalibrated = false);
	virtual ~GDF2();

	virtual double getSamplingFrequency() const override
	{
		return samplingFrequency;
	}
	virtual unsigned int getChannelCount() const override
	{
		return fh.numberOfChannels;
	}
	virtual uint64_t getSamplesRecorded() const override
	{
		return samplesRecorded;
	}
	virtual time_t getStartDate(int) const override
	{
		double fractionOfDay = ldexp(static_cast<double>(fh.startDate[0]), -32);
		double seconds = (fh.startDate[1] - 719529 + fractionOfDay)*24*60*60;
		return static_cast<time_t>(round(seconds));
	}
	virtual void save(DataModel* dataModel) override;
	virtual bool load(DataModel* dataModel) override;
	virtual void readSignalFromFile(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}
	virtual void readSignalFromFile(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) override
	{
		readSignalFromFileFloatDouble(dataChannels, firstSample, lastSample);
	}

private:
	char* recordRawBuffer;
	double* recordDoubleBuffer;

	std::fstream file;
	double samplingFrequency;
	uint64_t samplesRecorded;
	int64_t startOfData;
	int64_t startOfEventTable;
	double* scale;
	int dataTypeSize;
	std::function<double (void*)> convertSampleToDouble;
	int version;

	/**
	 * @brief A structure for storing values from the file's fixed header
	 * as C++ types at one place.
	 */
	struct
	{
		char versionID[8 + 1];
		char patientID[66 + 1];
		// 10B reserved
		char drugs;
		uint8_t weight;
		uint8_t height;
		char patientDetails;
		char recordingID[64 + 1];
		uint32_t recordingLocation[4];
		uint32_t startDate[2];
		uint32_t birthday[2];
		uint16_t headerLength;
		char ICD[6];
		uint64_t equipmentProviderID;
		// 6B reserved
		uint16_t headsize[3];
		float positionRE[3];
		float positionGE[3];
		int64_t numberOfDataRecords;
		char durationOfDataRecord[8];
		uint16_t numberOfChannels;
		// 2B reserved
	} fh;

	/**
	 * @brief A structure for storing values from the file's variable header
	 * as C++ types at one place.
	 */
	struct
	{
		char (* label)[16 + 1];
		char (* typeOfSensor)[80 + 1];
		// physicalDimension obsolete
		uint16_t* physicalDimensionCode;
		double* physicalMinimum;
		double* physicalMaximum;
		double* digitalMinimum;
		double* digitalMaximum;
		// prefiltering obsolete
		float* timeOffset;
		float* lowpass;
		float* highpass;
		float* notch;
		uint32_t* samplesPerRecord;
		uint32_t* typeOfData;
		float (* sensorPosition)[3];
		char (* sensorInfo)[20];
	} vh;

	template<typename T>
	void readSignalFromFileFloatDouble(std::vector<T*> dataChannels, const uint64_t firstSample, const uint64_t lastSample);
	void readGdfEventTable(DataModel* dataModel);
	void fillDefaultMontage(DataModel* dataModel);
};

} // namespace AlenkaFile

#endif // ALENKAFILE_GDF2_H
