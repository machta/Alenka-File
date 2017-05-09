#include <AlenkaFile/datafile.h>
#include <AlenkaFile/gdf2.h>
#include <AlenkaFile/edf.h>
#include <AlenkaFile/libgdf.h>
#include <AlenkaFile/mat.h>

#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>

using namespace std;
using namespace AlenkaFile;

const string TEST_DATA_PATH = "unit-test/data/";

const double MAX_REL_ERR_DOUBLE = 0.0001;
const double MAX_REL_ERR_FLOAT = 0.0001;
const double MAX_ABS_ERR_DOUBLE = 0.000001;
const double MAX_ABS_ERR_FLOAT = 0.01;

class TestFile
{
	bool hasValues = false;
	vector<double> values;

public:
	string path;
	double sampleRate;
	unsigned int channelCount, samplesRecorded;

	TestFile(const string& path, double sampleRate = 0, int channelCount = 0, int samplesRecorded = 0) :
		path(path), sampleRate(sampleRate), channelCount(channelCount), samplesRecorded(samplesRecorded) {}
	~TestFile() {}

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
	DataFile* makeMAT()
	{
		return new MAT(path + ".mat");
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
};

template<class T>
void fillVector(vector<T>& v, T val)
{
	for (auto& e : v)
		e = val;
}

template<class T, class U>
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

template<class T, class U>
void compareMatrix(T* arr, U* sol, int rows, int cols, double* relErr, double* absErr)
{
	compareMatrix(arr, sol, rows, cols, cols, cols, relErr, absErr);
}

template<class T, class U>
void compareMatrixAverage(T* arr, U* sol, int rows, int cols, int arrRowLen, int solRowLen, double* relErr, double* absErr)
{
	double rel = 0;
	double abs = 0;

	for (int j = 0; j < rows; j++)
	{
		for (int i = 0; i < cols; i++)
		{
			double diff = fabs(arr[i] - sol[i]);
			abs += diff;
			if (sol[i] != 0)
				rel += fabs(diff/sol[i]);
		}

		arr += arrRowLen;
		sol += solRowLen;
	}

	int count = rows*cols;
	*relErr = rel/count;
	*absErr = abs/count;
}

template<class T, class U>
void compareMatrixAverage(T* arr, U* sol, int rows, int cols, double* relErr, double* absErr)
{
	compareMatrixAverage(arr, sol, rows, cols, cols, cols, relErr, absErr);
}

inline void printException(function<void (void)> fun)
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
