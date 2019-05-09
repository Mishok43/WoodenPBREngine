#include "DOptions.h"

char* getCMDOption(char **begin, char** end, const std::string& option)
{
	char** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end)
	{
		return *itr;
	}
	return nullptr;
}

bool cmdOptionExists(char** begin, char ** end, const std::string& option)
{
	return std::find(begin, end, option) != end;
}

WPBR_BEGIN
DOptions DOptions::createOptionsByCMDArguments(char* argv[], std::size_t numArguments)
{
	DOptions options;

	char** argBegin = argv;
	char** argEnd = argv + numArguments;

	{   
		char* filename = getCMDOption(argBegin, argEnd, "-f");
		if (filename)
		{
			options.inSceneFileName = filename;
		}
	}
	
}
WPBR_END