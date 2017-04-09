#include <gtest/gtest.h>

#include <AlenkaFile/gdf2.h>
#include <AlenkaFile/edf.h>
#include <AlenkaFile/libgdf.h>

#include <fstream>
#include <string>
#include <memory>

using namespace std;
using namespace AlenkaFile;

namespace
{

template <class T>
void fillVector(vector<T>& v, T val)
{
	for (auto& e : v)
		e = val;
}

template <class T, class U>
void compareMatrix(T* arr, U* sol, int rows, int cols, int arrRowLen, int solRowLen, double* relErr, double* absErr)
{
	double rel = 0;
	double abs = 0;

	for (int j = 0; j < rows; j++)
	{
		for (int i = 0; i < cols; i++)
		{
			double diff = fabs(arr[i] - sol[i]);
			abs = max<double>(abs, diff);
			if (sol[i] != 0)
				rel = max<double>(rel, fabs(diff/sol[i]));
		}

		arr += arrRowLen;
		sol += solRowLen;
	}

	*relErr = rel;
	*absErr = abs;
}

template <class T, class U>
void compareMatrix(T* arr, U* sol, int rows, int cols, double* relErr, double* absErr)
{
	compareMatrix(arr, sol, rows, cols, cols, cols, relErr, absErr);
}

const double MAX_REL_ERR_DOUBLE = 0.0001;
const double MAX_REL_ERR_FLOAT = 0.0001;
const double MAX_ABS_ERR_DOUBLE = 0.000001;
const double MAX_ABS_ERR_FLOAT = 0.01;

void printException(function<void (void)> fun)
{
	try
	{
		fun();
	}
	catch (exception& e)
	{
		cerr << "Caught an std exception: " << e.what() << endl;
		throw;
	}
	catch (...)
	{
		cerr << "Caught an exception." << endl;
		throw;
	}
}

} // namespace

class testFile
{
	bool hasValues = false;
	vector<double> values;

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
		return new GDF2(path + ".gdf");
	}

	DataFile* makeEDF()
	{
		return new EDF(path + ".edf");
	}

	DataFile* makeLibGDF()
	{
		return new LibGDF(path + ".gdf");
	}

	const vector<double>& getValues()
	{
		if (hasValues == false)
		{
			hasValues = true;

			fstream valuesFile;
			valuesFile.exceptions(ifstream::failbit | ifstream::badbit);
			valuesFile.open(path + "_values.dat");

			for (unsigned int i = 0; i < channelCount; i++)
			{
				for (unsigned int j = 0; j < samplesRecorded; j++)
				{
					double sample;
					valuesFile >> sample;
					values.push_back(sample);
				}
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

	void outOfBoundsTest(DataFile* file, int nNormal, int nZero)
	{
		vector<double> a((nNormal + nZero)*file->getChannelCount());
		vector<double> b(nNormal*file->getChannelCount());
		vector<double> zero(nZero*file->getChannelCount(), 0);

		// Read partially before.
		fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));
		fillVector(b, static_cast<double>(0xBBBBBBBBBBBBBBBB));

		file->readSignal(a.data(), -nZero, nNormal - 1);
		file->readSignal(b.data(), 0, nNormal - 1);

		double relErr, absErr;

		compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero, nNormal + nZero, nZero, &relErr, &absErr);
		EXPECT_DOUBLE_EQ(relErr, 0);
		EXPECT_DOUBLE_EQ(absErr, 0);

		compareMatrix(a.data() + nZero, b.data(), file->getChannelCount(), nNormal, nNormal + nZero, nNormal, &relErr, &absErr);
		EXPECT_DOUBLE_EQ(relErr, 0);
		EXPECT_DOUBLE_EQ(absErr, 0);

		// Read partially after.
		fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));
		fillVector(b, static_cast<double>(0xBBBBBBBBBBBBBBBB));

		size_t last = file->getSamplesRecorded() - 1;
		file->readSignal(a.data(), last - nNormal + 1, last + nZero);
		file->readSignal(b.data(), last - nNormal + 1, last);

		compareMatrix(a.data() + nNormal, zero.data(), file->getChannelCount(), nZero, nNormal + nZero, nZero, &relErr, &absErr);
		EXPECT_DOUBLE_EQ(relErr, 0);
		EXPECT_DOUBLE_EQ(absErr, 0);

		compareMatrix(a.data(), b.data(), file->getChannelCount(), nNormal, nNormal + nZero, nNormal, &relErr, &absErr);
		EXPECT_DOUBLE_EQ(relErr, 0);
		EXPECT_DOUBLE_EQ(absErr, 0);

		// Read only before.
		if (nZero > 0)
		{
			fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));

			file->readSignal(a.data(), -1000 -nZero - nNormal + 1, -1000 -nNormal);

			compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero, &relErr, &absErr);
			EXPECT_DOUBLE_EQ(relErr, 0);
			EXPECT_DOUBLE_EQ(absErr, 0);
		}

		// Read only after.
		if (nZero > 0)
		{
			fillVector(a, static_cast<double>(0xAAAAAAAAAAAAAAAA));

			auto end = 1000 + file->getSamplesRecorded();
			file->readSignal(a.data(), end + nNormal, end + nNormal + nZero - 1);

			compareMatrix(a.data(), zero.data(), file->getChannelCount(), nZero, &relErr, &absErr);
			EXPECT_DOUBLE_EQ(relErr, 0);
			EXPECT_DOUBLE_EQ(absErr, 0);
		}
	}

	void outOfBoundsTest(DataFile* file)
	{
		outOfBoundsTest(file, 1, 0);
		outOfBoundsTest(file, 100, 0);

		vector<int> valuesToTry = {29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 100, 101, 149, 200, 229, 1111};

		for (int i = 1; i <= 25; i++)
			valuesToTry.push_back(i);

		for (int e : valuesToTry)
			for (int ee : valuesToTry)
				outOfBoundsTest(file, e, ee);
	}

	void dataTest(DataFile* file, double maxRelErrDouble = MAX_REL_ERR_DOUBLE, double maxRelErrFloat = MAX_REL_ERR_FLOAT,
				  double maxAbsErrDouble = MAX_ABS_ERR_DOUBLE, double maxAbsErrFloat = MAX_ABS_ERR_FLOAT)
	{
		int n = channelCount*samplesRecorded;

		vector<double> dataD;
		dataD.insert(dataD.begin(), n, 0);
		file->readSignal(dataD.data(), 0, samplesRecorded - 1);

		vector<float> dataF;
		dataF.insert(dataF.begin(), n, 0);
		file->readSignal(dataF.data(), 0, samplesRecorded - 1);

		double relErr, absErr;

		compareMatrix(dataD.data(), getValues().data(), channelCount, samplesRecorded, &relErr, &absErr);
		EXPECT_LT(relErr, maxRelErrDouble);
		EXPECT_LT(absErr, maxAbsErrDouble);

		compareMatrix(dataF.data(), getValues().data(), channelCount, samplesRecorded, &relErr, &absErr);
		EXPECT_LT(relErr, maxRelErrFloat);
		EXPECT_LT(absErr, maxAbsErrFloat);
	}
};

class primary_file_test : public ::testing::Test
{
protected:
	primary_file_test() :
		gdf00(path + "gdf/gdf00", 200, 19, 364000),
		gdf01(path + "gdf/gdf01", 50, 1, 2050),
		edf00(path + "edf/edf00", 200, 37, 363620)
	{}

	virtual ~primary_file_test()
	{}

	template<class T>
	void gdfStartTimeTest()
	{
		unique_ptr<DataFile> file;

		ASSERT_NO_THROW(file.reset(new T(path + "gdf/startTime.gdf")));

		time_t time = file->getStartDate();
		time_t seconds = 1231276659; // Seconds between 1970 and Tue Jan  6 21:17:39 2009
		EXPECT_EQ(time, seconds);
	}

	string path = "unit-test/data/";

	testFile gdf00, gdf01;
	testFile edf00;
};

// Tests common to all file types.
TEST_F(primary_file_test, outOfBounds)
{
	gdf00.outOfBoundsTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
	gdf01.outOfBoundsTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());

	gdf00.outOfBoundsTest(unique_ptr<DataFile>(gdf00.makeLibGDF()).get());
	gdf01.outOfBoundsTest(unique_ptr<DataFile>(gdf01.makeLibGDF()).get());
	
	edf00.outOfBoundsTest(unique_ptr<DataFile>(edf00.makeEDF()).get());
}

// TODO: Add all kinds of crazy tests that read samples and compare them to data read from the whole file. Like read only one sample long block.
// TODO: Test whether readSignal modifies immediately before and after the bufer size -- whether it writes out of bounds.

// Tests of my GDF implementation.
TEST_F(primary_file_test, GDF2_exceptions)
{
	EXPECT_ANY_THROW(printException([this] () { GDF2 file(path + "gdf/empty.gdf"); }));
	EXPECT_ANY_THROW(printException([this] () { GDF2 file(path + "gdf/headerOnly.gdf"); }));
	EXPECT_THROW(printException([this] () { GDF2 file(path + "gdf/badType.gdf"); }), runtime_error);
	EXPECT_THROW(printException([this] () { GDF2 file(path + "gdf/badFile.gdf"); }), runtime_error);

	unique_ptr<DataFile> file;

	vector<double> data;
	data.insert(data.begin(), 100000, 0);

	ASSERT_NO_THROW(file.reset(new GDF2(path + "gdf/gdf00.gdf")));
	EXPECT_THROW(printException([this, &file, &data] () { file->readSignal(data.data(), 100, 50); }), invalid_argument);
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

TEST_F(primary_file_test, GDF2_data_00) // TODO: generate new values files with higher precision
{
	gdf00.dataTest(unique_ptr<DataFile>(gdf00.makeGDF2()).get());
}

TEST_F(primary_file_test, GDF2_data_01)
{
	gdf01.dataTest(unique_ptr<DataFile>(gdf01.makeGDF2()).get());
}

// Tests of LibGDF.
TEST_F(primary_file_test, LibGDF_exceptions)
{
	//EXPECT_ANY_THROW(printException([this] () { LibGDF file(path + "gdf/empty.gdf"); }));
	//EXPECT_ANY_THROW(printException([this] () { LibGDF file(path + "gdf/headerOnly.gdf"); }));
	// These two tests halt on assertions. Would it be better if exceptions were thrown instead?

	EXPECT_ANY_THROW(printException([this] () { LibGDF file(path + "gdf/badType.gdf"); }));
	EXPECT_ANY_THROW(printException([this] () { LibGDF file(path + "gdf/badFile.gdf"); }));

	unique_ptr<DataFile> file;

	vector<double> data;
	data.insert(data.begin(), 100000, 0);

	ASSERT_NO_THROW(file.reset(new LibGDF(path + "gdf/gdf00.gdf")));
	EXPECT_THROW(printException([this, &file, &data] () { file->readSignal(data.data(), 100, 50); }), invalid_argument);
}

TEST_F(primary_file_test, LibGDF_startTime)
{
	gdfStartTimeTest<LibGDF>();
}

TEST_F(primary_file_test, LibGDF_metaInfo)
{
	gdf00.metaInfoTest(unique_ptr<DataFile>(gdf00.makeLibGDF()).get());
	gdf01.metaInfoTest(unique_ptr<DataFile>(gdf01.makeLibGDF()).get());
}

TEST_F(primary_file_test, LibGDF_data_00)
{
	gdf00.dataTest(unique_ptr<DataFile>(gdf00.makeLibGDF()).get());
}

TEST_F(primary_file_test, LibGDF_data_01)
{
	gdf01.dataTest(unique_ptr<DataFile>(gdf01.makeLibGDF()).get());
}

// Tests of EDFlib.
TEST_F(primary_file_test, EDF_exceptions)
{
	unique_ptr<DataFile> file;

	vector<double> data;
	data.insert(data.begin(), 100000, 0);

	ASSERT_NO_THROW(file.reset(new EDF(path + "edf/edf00.edf")));
	EXPECT_THROW(printException([this, &file, &data] () { file->readSignal(data.data(), 100, 50); }), invalid_argument);
}

TEST_F(primary_file_test, EDF_startTime)
{
	unique_ptr<DataFile> file;

	ASSERT_NO_THROW(file.reset(new EDF(path + "edf/edf00.edf")));

	time_t time = file->getStartDate();
	time_t seconds = 1126779522; // Seconds between 1970 and Thu, 15 Sep 2005 10:18:42 GMT
	EXPECT_LE(time - seconds, 60*60*24) << "Start time is within 24 hours.";
}

TEST_F(primary_file_test, EDF_metaInfo)
{
	edf00.metaInfoTest(unique_ptr<DataFile>(edf00.makeEDF()).get());
}

TEST_F(primary_file_test, EDF_data_00)
{
	edf00.dataTest(unique_ptr<DataFile>(edf00.makeEDF()).get(), MAX_REL_ERR_DOUBLE/10000,
				   MAX_REL_ERR_FLOAT/1000, MAX_ABS_ERR_DOUBLE/100000, MAX_ABS_ERR_FLOAT/100);
}

// TODO: add a small edf file to test; like the gdf01
