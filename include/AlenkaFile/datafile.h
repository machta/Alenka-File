#ifndef ALENKAFILE_DATAFILE_H
#define ALENKAFILE_DATAFILE_H

#include "AlenkaFile/abstractdatamodel.h"

#include <cstdint>
#include <string>
#include <vector>
#include <cassert>

// TODO: add a public interface for access to the headers
// TODO: upgrade some of the asserts to exceptions

namespace AlenkaFile
{

/**
 * @brief An abstract base class of the data files.
 *
 * DataFile implements all the operations concerning the secondary files --
 * extensions of the primary files. It is assumed that the primary and secondary
 * file is always in the same directory, and the secondary file's name is
 * primaryFileName.info (e.g. primary file is 'sample.gdf' and secondary is
 * 'sample.gdf.info'.
 *
 * To implement a new file type you need to subclass DataFile and implement
 * the all the pure virtual functions to provide the implementation of the most
 * basic file-type specific functionality.
 *
 * It is assumed that every channel has the same sampling frequency and the same total
 * number of samples recorded.
 */
class DataFile
{
public:
	/**
	 * @brief DataFile constructor.
	 * @param filePath The file path of the primary file.
	 */
	DataFile(const std::string& filePath) : filePath(filePath) {}
	virtual ~DataFile() {}

	std::string getFilePath() const
	{
		return filePath;
	}

	/**
	 * @brief Returns the sampling frequency of the stored signal.
	 */
	virtual double getSamplingFrequency() const = 0;

	/**
	 * @brief Returns the number of channels of the stored signal.
	 */
	virtual unsigned int getChannelCount() const = 0;

	/**
	 * @brief Return the total number of samples recorded for each channel.
	 */
	virtual uint64_t getSamplesRecorded() const = 0;

	/**
	 * @brief Returns the date and time of the start of recording.
	 *
	 * Some implementations depend on the system time zone,
	 * so you can manually specify the current time zone to get the universal time.
	 * For example Prague, Czech Republic, is one hour ahead, so pass 1 to get UTC.
	 */
	virtual time_t getStartDate(int timeZone = 0) const = 0;

	/**
	 * @brief Saves the .info file.
	 *
	 * @param infoFile [out]
	 */
	virtual void saveSecondaryFile(std::string montFilePath = "");
	virtual void save()
	{
		saveSecondaryFile();
	}

	/**
	 * @brief Loads the information from the .info file.
	 * @return True if the .info file was located.
	 *
	 * This method tries to load the necessary information from the .info file.
	 * If the file is not located, false is returned. The extending class can then
	 * load this information instead from the primary file.
	 *
	 * An empty montage is always created.???
	 * @param infoFile [in]
	 */
	virtual bool loadSecondaryFile(std::string montFilePath = "");
	virtual bool load()
	{
		return loadSecondaryFile();;
	}
	// TODO: Make sure load can be called repeatedly with the same results.

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range can exceed the boundaries of the signal data stored in the file.
	 * These extra samples are set to zero.
	 *
	 * The actual reading from the primary file is delegated to readChannels.
	 * @param data [out] The signal data is copied to this buffer.
	 * @param firstSample The first sample to be read.
	 * @param lastSample The last sample to be read.
	 */
	void readSignal(float* data, int64_t firstSample, int64_t lastSample);

	/**
	 * \overload void readSignal(float* data, int64_t firstSample, int64_t lastSample)
	 */
	void readSignal(double* data, int64_t firstSample, int64_t lastSample);

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range cannot exceed the boundaries of the signal.
	 * @param dataChannels A vector of pointers to the beginning of channels.
	 * @param firstSample The first sample to be read.
	 * @param lastSample The last sample to be read.
	 */
	virtual void readChannels(std::vector<float*> dataChannels, uint64_t firstSample, uint64_t lastSample) = 0;

	/**
	 * \overload virtual void readChannels(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) = 0
	 */
	virtual void readChannels(std::vector<double*> dataChannels, uint64_t firstSample, uint64_t lastSample) = 0;

	DataModel* getDataModel() const
	{
		assert(dataModel->montageTable() && dataModel->eventTypeTable());
		return dataModel;
	}
	void setDataModel(DataModel* dataModel)
	{
		this->dataModel = dataModel;
	}

	virtual double getPhysicalMaximum(unsigned int channel) { return 0; (void)channel; }
	virtual double getPhysicalMinimum(unsigned int channel) { return 0; (void)channel;}
	virtual double getDigitalMaximum(unsigned int channel) { return 32767; (void)channel;}
	virtual double getDigitalMinimum(unsigned int channel) { return -32768; (void)channel;}
	virtual std::string getLabel(unsigned int channel) { return ""; (void)channel; }

	/**
	 * @brief Tests endianness.
	 * @return Returns true if this computer is little-endian, false otherwise.
	 */
	static bool testLittleEndian()
	{
		unsigned int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes == 1;
	}

	/**
	 * @brief Reverse endianness of data.
	 * @param data [in,out]
	 * @param size Size in bytes.
	 */
	static void changeEndianness(char* data, int size)
	{
		for (int i = 0, sizeHalf = size/2; i < sizeHalf; ++i)
			std::swap(data[i], data[size - i - 1]);
	}

	/**
	 * @brief Reverse endianness of val.
	 * @param val [in,out]
	 */
	template<typename T>
	static void changeEndianness(T* val)
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}

private:
	std::string filePath;
	DataModel* dataModel;
};

} // namespace AlenkaFile

#endif // ALENKAFILE_DATAFILE_H
