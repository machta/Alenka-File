#ifndef DATAFILE_H
#define DATAFILE_H

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief An abstract base class of the data files.
 *
 * DataFile implements the operations concerning .mont and .event files that
 * are common for all data file types.
 *
 * To implement a new file type you need to subclass this type and implement
 * the pure virtual functions.
 */
class DataFile
{
public:
	/**
	 * @brief DataFile constructor.
	 * @param filePath The file path of the data file without the extension.
	 */
	DataFile(const std::string& filePath);
	virtual ~DataFile();

	/**
	 * @brief Returns the sampling frequency of the stored signal.
	 */
	virtual double getSamplingFrequency() const = 0;

	/**
	 * @brief Returns the number of channels of the stored signal.
	 */
	virtual unsigned int getChannelCount() const = 0;

	/**
	 * @brief Return the total number of samples recorded for one channel.
	 */
	virtual uint64_t getSamplesRecorded() const = 0;

	/**
	 * @brief Returns the date of the start of recording.
	 */
	virtual time_t getStartDate() const = 0;

	/**
	 * @brief Saves the .mont file.
	 */
	virtual void save();

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range can exceed the boundaries of the signal data stored in the file.
	 * These extra samples are set to zero.
	 * @param data [out] The signal data is stored in this vector.
	 * @param firstSample The first sample to be loaded.
	 * @param lastSample The last sample to be loaded.
	 */
	virtual void readData(std::vector<float>* data, int64_t firstSample, int64_t lastSample) = 0;

	/**
	 * @brief Reads signal data specified by the sample range.
	 *
	 * The range can exceed the boundaries of the signal data stored in the file.
	 * These extra samples are set to zero.
	 * @param data [out] The signal data is stored in this vector.
	 * @param firstSample The first sample to be loaded.
	 * @param lastSample The last sample to be loaded.
	 */
	virtual void readData(std::vector<double>* data, int64_t firstSample, int64_t lastSample) = 0;

protected:
	/**
	 * @brief Loads the info from the .mont file.
	 * @return True if the .mont file was located.
	 *
	 * This method tries to load the necessary information from the .mont file.
	 * If the file is not located false is returned. The extending class then
	 * can load this information instead in its specific way.
	 *
	 * An empty montage is always created.
	 */
	virtual bool load();

	/**
	 * @brief Tests endianness.
	 * @return Returns true if this computer is little-endian, false otherwise.
	 */
	bool testLittleEndian() const
	{
		unsigned int number = 1;
		char* bytes = reinterpret_cast<char*>(&number);
		return *bytes == 1;
	}

	/**
	 * @brief Reverse endianness.
	 * @param data [in,out]
	 * @param size Size in bytes.
	 */
	void changeEndianness(char* data, int size) const
	{
		for (int i = 0, sizeHalf = size/2; i < sizeHalf; ++i)
		{
			std::swap(data[i], data[size - i - 1]);
		}
	}

	/**
	 * @brief Reverse endianness.
	 * @param val [in,out]
	 */
	template<typename T>
	void changeEndianness(T* val) const
	{
		changeEndianness(reinterpret_cast<char*>(val), sizeof(T));
	}

private:
	std::string filePath;
};

#endif // DATAFILE_H
