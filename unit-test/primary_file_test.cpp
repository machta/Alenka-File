#include "gtest/gtest.h"

#include "gdf2.h"

#include <string>
#include <vector>
#include <fstream>

using namespace std;

class testFile
{
	bool hasValues = false;
	vector<vector<double>> values;

public:
	string path;
	double sampleRate;
	unsigned int channelCount, samplesRecorded;

	testFile(const string& path, double sampleRate, int channelCount, int samplesRecorded) :
		path(path), sampleRate(sampleRate), channelCount(channelCount), samplesRecorded(samplesRecorded)
	{}

	~testFile()
	{}

	DataFile* makeGDF2()
	{
		return new GDF2(path);
	}

	const vector<vector<double>>& getValues()
	{
		if (hasValues == false)
		{
			hasValues = true;
			fstream valuesFile(path + "_values.dat");

			for (unsigned int i = 0; i < channelCount; i++)
			{
				vector<double> channel;
				for (unsigned int j = 0; j < samplesRecorded; j++)
				{
					double sample;
					valuesFile >> sample;
					channel.push_back(sample);
				}
				values.push_back(channel);
			}
		}

		return values;
	}

	void metaInfoTest(DataFile* file)
	{
		EXPECT_DOUBLE_EQ(file->getSamplingFrequency(), sampleRate);
		EXPECT_EQ(file->getChannelCount(), channelCount);
		EXPECT_EQ(file->getSamplesRecorded(), samplesRecorded);
	}

	void outOfBoundsTest(DataFile* file)
	{
		int nNormal = 149;
		int nZero = 229;

		vector<double> a;
		a.insert(a.begin(), (nNormal + nZero)*file->getChannelCount(), 0);

		vector<double> b;
		b.insert(b.begin(), nNormal*file->getChannelCount(), 0);

		file->readData(&a, -nZero, nNormal - 1);
		file->readData(&b, 0, nNormal - 1);

		for (int i = 0; i < nZero; ++i)
		{
			for (unsigned int j = 0; j < file->getChannelCount(); ++j)
			{
				EXPECT_DOUBLE_EQ(a[(nNormal + nZero)*j + i], 0);
			}
		}

		for (int i = 0; i < nNormal; ++i)
		{
			for (unsigned int j = 0; j < file->getChannelCount(); ++j)
			{
				EXPECT_DOUBLE_EQ(a[(nNormal + nZero)*j + nZero + i], b[nNormal*j + i]);
			}
		}

		int last = file->getSamplesRecorded() - 1;
		file->readData(&a, last - nNormal + 1, last + nZero);
		file->readData(&b, last - nNormal + 1, last);

		for (int i = 0; i < nZero; ++i)
		{
			for (unsigned int j = 0; j < file->getChannelCount(); ++j)
			{
				EXPECT_DOUBLE_EQ(a[(nNormal + nZero)*j + nNormal + i], 0);
			}
		}

		for (int i = 0; i < nNormal; ++i)
		{
			for (unsigned int j = 0; j < file->getChannelCount(); ++j)
			{
				EXPECT_DOUBLE_EQ(a[(nNormal + nZero)*j + i], b[nNormal*j + i]);
			}
		}
	}

	void dataTest(DataFile* file)
	{
		int n = channelCount*samplesRecorded;

		vector<double> dataD;
		dataD.insert(dataD.begin(), n, 0);
		file->readData(&dataD, 0, samplesRecorded - 1);

		vector<float> dataF;
		dataF.insert(dataF.begin(), n, 0);
		file->readData(&dataF, 0, samplesRecorded - 1);

		for (unsigned int i = 0; i < channelCount; i++)
		{
			for (unsigned int j = 0; j < samplesRecorded; j++)
			{
				double value = getValues()[i][j];

				EXPECT_NEAR(value, dataD.at(i*samplesRecorded + j), 0.000001) << "Double test failed.";
				EXPECT_NEAR(value, dataF.at(i*samplesRecorded + j), 0.01) << "Float test failed.";
			}
		}
	}
};

class primary_file_test : public ::testing::Test
{
protected:
	primary_file_test() :
		gdf00(path + "gdf/gdf00", 200, 19, 364000),
		gdf01(path + "gdf/gdf01", 50, 1, 2050)
	{}

	virtual ~primary_file_test()
	{}

	template<class T>
	void gdfExceptionsTest()
	{
		EXPECT_THROW(T file(path + "gdf/empty"), runtime_error);
		EXPECT_THROW(T file(path + "gdf/headerOnly"), runtime_error);
		EXPECT_THROW(T file(path + "gdf/badType"), runtime_error);
		EXPECT_THROW(T file(path + "gdf/badFile"), runtime_error);

		unique_ptr<DataFile> file;

		vector<double> data;
		data.insert(data.begin(), 100000, 0);

		ASSERT_NO_THROW(file.reset(new T(path + "gdf/gdf00")));
		EXPECT_THROW(file->readData(&data, 100, 50), invalid_argument);
	}

	template<class T>
	void gdfStartTimeTest()
	{
		unique_ptr<DataFile> file;

		ASSERT_NO_THROW(file.reset(new T(path + "gdf/startTime")));

		time_t time = file->getStartDate();
		time_t seconds = 1231276659; // Seconds between 1970 and Tue Jan  6 21:17:39 2009
		EXPECT_EQ(time, seconds);
	}

	string path = "unit-test/data/";

	testFile gdf00, gdf01;
};

// Tests commot for all types.
TEST_F(primary_file_test, outOfBounds)
{
	gdf00.outOfBoundsTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
	gdf01.outOfBoundsTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());
}

// Tests of GDF files.
TEST_F(primary_file_test, GDF2_exceptions)
{
	gdfExceptionsTest<GDF2>();
}

TEST_F(primary_file_test, GDF2_startTime)
{
	gdfStartTimeTest<GDF2>();
}

TEST_F(primary_file_test, GDF2_metaInfo)
{
	gdf00.metaInfoTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
	gdf01.metaInfoTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());
}

TEST_F(primary_file_test, GDF2_data_t00)
{
	gdf00.dataTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
}

TEST_F(primary_file_test, GDF2_data_t01)
{
	gdf01.dataTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());
}

// Tests of EDF files.
