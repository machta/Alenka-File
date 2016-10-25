#include "libgdf.h"

#include <GDF/Reader.h>

using namespace std;
using namespace gdf;

LibGDF::LibGDF(const string& filePath) : DataFile(filePath)
{
	gdfReader = new Reader();
	gdfReader->open(filePath + ".gdf");

	//assert(file.is_open() && "File GDF2 was not successfully opened.");


}

LibGDF::~LibGDF()
{

}

time_t LibGDF::getStartDate(int timeZone) const
{
	return 0;
}

void LibGDF::save()
{

}

bool LibGDF::load()
{
	return true;
}
