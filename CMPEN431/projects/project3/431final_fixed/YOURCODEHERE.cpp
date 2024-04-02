#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <math.h>
#include <fcntl.h>
#include <vector>
#include <iterator>

#include "431project.h"

using namespace std;

/*
 * Enter your PSU IDs here to select the appropriate scanning order.
 */
//my psu id mod 24 = 1, which follows BPs, Chche, FPU, Core
#define PSU_ID_SUM (962361481)

/*
 * Some global variables to track heuristic progress.
 * 
 * Feel free to create more global variables to track progress of your
 * heuristic.
 */
unsigned int currentlyExploringDim = 0 ;
bool currentDimDone = false;
bool isDSEComplete = false;
int visit2 = 0;
std::string prevConfig;
/*
 * Given a half-baked configuration containing cache properties, generate
 * latency parameters in configuration string. You will need information about
 * how different cache paramters affect access latency.
 * 
 * Returns a string similar to "1 1 1"
 */
std::string generateCacheLatencyParams(string halfBackedConfig) {

	string latencySettings;
	//
	//YOUR CODE BEGINS HERE
	int il1size = getil1size(halfBackedConfig);
	int dl1size = getdl1size(halfBackedConfig);
	int ul2size = getl2size(halfBackedConfig);
	int il1assoc_num = pow(2, extractConfigParam(halfBackedConfig, 6));
	int dl1assoc_num = pow(2, extractConfigParam(halfBackedConfig, 4));
	int ul2assoc_num = pow(2, extractConfigParam(halfBackedConfig, 9));
	int il1lat_index = log2(il1assoc_num) + log2(il1size) - 10 - 1;
	int dl1lat_index = log2(dl1assoc_num) + log2(dl1size) - 10 - 1;
	int ul2lat_index = log2(ul2assoc_num) + log2(ul2size) - 10 - 5;
	// Replace this dumb implementation.
	std::stringstream ss;
	ss << dl1lat_index << " " << il1lat_index << " " << ul2lat_index;
	latencySettings = ss.str();
	//
	//YOUR CODE ENDS HERE
	//
	return latencySettings;
}

/*
 * Returns 1 if configuration is valid, else 0
 */
int validateConfiguration(std::string configuration) {

	// FIXME - YOUR CODE HERE
	unsigned int l1block_size = pow(2, 3 + extractConfigParam(configuration, 2));
	unsigned int ul2block_size = pow(2, 4 + extractConfigParam(configuration, 8));
	unsigned int width = pow(2, extractConfigParam(configuration, 0));
	//l1 block size should be at least 8Bytes
	if(l1block_size < width * 8){
		return 0;
	}
	//The  ul2  (unified  L2  cache)  block  size  must  be  at  least  twice  your  il1  (and  dl1) block size with a maximum block size of 128B.
	if(ul2block_size < (2 * l1block_size) || ul2block_size > 128){
		return 0;
	}
	//cache size of ul2 need to be least twice as large as il1+dl1 in order to be inclusive.
	if(getl2size(configuration) < 2 * (getdl1size(configuration) + getil1size(configuration))){
		return 0;
	}
	//check if il1 cahce size is valid
	if(getil1size(configuration) < 2 * 1024 || getil1size(configuration) > 64 * 1024){
		return 0;
	}
	//check if dl1 cahce size is valid
	if(getdl1size(configuration) < 2 * 1024 || getdl1size(configuration) > 64 * 1024){
		return 0;
	}
	//check if ul2 cahce size is valid
	if(getl2size(configuration) < 32 * 1024 || getl2size(configuration) > 1024 * 1024){
		return 0;
	}
	//isNumDimConfiguration will return 1 for valid number of config params, 0 for fail
	return isNumDimConfiguration(configuration);
}

//convert original configration to the order that I want to search
std::string convertMyOrderConfig(std::string configuration){
	std::stringstream ss;
	
	//put BP first
	for(int dim = 12; dim <= 14; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put cache second
	for(int dim = 2; dim <= 10; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put FPU third
	ss << extractConfigParam(configuration, 11) << " ";
	//put Core forth
	for(int dim = 0; dim <= 1; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put latency last
	for(int dim = 15; dim <= 17; dim++){
		ss << extractConfigParam(configuration, dim) << " ";	
	}
	string MyOrderConfig = ss.str();
	return MyOrderConfig;
}

//revert the configuration from my order abck to original one
std::string revertMyOrderConfig(std::string configuration){
	std::stringstream ss;
	//put Core first
	for(int dim = 13; dim <= 14; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put cache second
	for(int dim = 3; dim <= 11; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put FPU third
	ss << extractConfigParam(configuration, 12) << " ";
	//put BP forth
	for(int dim = 0; dim <= 2; dim++){
		ss << extractConfigParam(configuration, dim) << " ";
	}
	//put latency last
	// for(int dim = 15; dim <= 17; dim++){
	// 	ss << extractConfigParam(configuration, dim) << " ";	
	// }
	string revertConfig = ss.str();
	return revertConfig;
}

/*
 * Given the current best known configuration, the current configuration,
 * and the globally visible map of all previously investigated configurations,
 * suggest a previously unexplored design point. You will only be allowed to
 * investigate 1000 design points in a particular run, so choose wisely.
 *
 * In the current implementation, we start from the leftmost dimension and
 * explore all possible options for this dimension and then go to the next
 * dimension until the rightmost dimension.
 */
std::string generateNextConfigurationProposal(std::string currentconfiguration,
		std::string bestEXECconfiguration, std::string bestEDPconfiguration,
		int optimizeforEXEC, int optimizeforEDP) {

	//
	// Some interesting variables in 431project.h include:
	//
	// 1. GLOB_dimensioncardinality
	// 2. GLOB_baseline
	// 3. NUM_DIMS
	// 4. NUM_DIMS_DEPENDENT
	// 5. GLOB_seen_configurations
	std::string nextconfiguration = currentconfiguration;
	// Continue if proposed configuration is invalid or has been seen/checked before.
	while (!validateConfiguration(nextconfiguration) ||
		GLOB_seen_configurations[nextconfiguration]) {
		// Check if DSE has been completed before and return current
		// configuration.
		if(isDSEComplete) {
			return currentconfiguration;
		}

		std::stringstream ss;

		string bestConfig;
		if (optimizeforEXEC == 1)
			bestConfig = bestEXECconfiguration;

		if (optimizeforEDP == 1)
			bestConfig = bestEDPconfiguration;

		//convert bestConfig and nextconfiguration to my order
		string Myorder_bestConfig = convertMyOrderConfig(bestConfig);
		string Myorder_nextconfiguration = convertMyOrderConfig(nextconfiguration);

		// Fill in the dimensions already-scanned with the already-selected best
		// value.
		string Myorder_GLOB_baseline = convertMyOrderConfig(GLOB_baseline);

		for (int dim = 0; dim < currentlyExploringDim; dim++) {
			ss << extractConfigParam(Myorder_bestConfig, dim) << " ";
		}

	
		// Handling for currently exploring dimension. This is a very dumb
		// implementation.
		int nextValue = extractConfigParam(Myorder_nextconfiguration, currentlyExploringDim) + 1;

		if(extractConfigParam(Myorder_GLOB_baseline, currentlyExploringDim) == 0){
			if (nextValue >= Myorder_GLOB_dimensioncardinality[currentlyExploringDim]) {
				nextValue = Myorder_GLOB_dimensioncardinality[currentlyExploringDim] - 1;
				currentDimDone = true;
			}
		}
		else{
			if (nextValue >= Myorder_GLOB_dimensioncardinality[currentlyExploringDim]) {
				nextValue = 0;
			}
			if(nextValue == extractConfigParam(Myorder_GLOB_baseline, currentlyExploringDim) - 1){
				currentDimDone = true;
			}
		}

		
		ss << nextValue << " ";
		
		// Fill in remaining independent params with 0.

		for (int dim = (currentlyExploringDim + 1);
				dim < (NUM_DIMS - NUM_DIMS_DEPENDENT); dim++) {
			ss << extractConfigParam(Myorder_GLOB_baseline, dim) << " ";
		}

		//
		// Last NUM_DIMS_DEPENDENT3 configuration parameters are not independent.
		// They depend on one or more parameters already set. Determine the
		// remaining parameters based on already decided independent ones.
		//

		string configSoFar = ss.str();
		configSoFar = revertMyOrderConfig(configSoFar);

		// Populate this object using corresponding parameters from config.
		std::stringstream ss2;
		ss2 << configSoFar;
		
		ss2 << generateCacheLatencyParams(configSoFar);

		// Configuration is ready now.
		nextconfiguration = ss2.str();
		
		// Make sure we start exploring next dimension in next iteration.
		if (currentDimDone) {
			currentlyExploringDim++;
			currentDimDone = false;
		}
		
		// Signal that DSE is complete after this configuration.
		if (currentlyExploringDim == (NUM_DIMS - NUM_DIMS_DEPENDENT))
			isDSEComplete = true;
	}
	return nextconfiguration;
}

