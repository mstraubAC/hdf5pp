/*
 * Dataset.cpp
 *
 *  Created on: Jul 18, 2013
 *      Author: marcel
 */

#include "Dataset.h"
#include "Exception.h"
#include <iostream>
#include "hdfLLReading.h"

using namespace std;
namespace hdf5
{

	Dataset::Dataset()
	{
		// TODO Auto-generated constructor stub
		fType = ObjectType::Dataset;
	}

	Dataset::~Dataset()
	{
		H5Sclose(fSpace);
		H5Dclose(fObjectId);
	}

	size_t Dataset::getRank() const
	{
		int a = H5Sget_simple_extent_ndims(fSpace);
		if (a < 0) {
			throw Exception("Could not get dimensionality of dataset");
		}
		return static_cast<size_t>(a);
	}

	size_t Dataset::getDimension(size_t dim) const
	{
		if (dim >= getRank()) {
			throw Exception("The requested dimension is out-of-range");
		}

		hsize_t* dims = new hsize_t[getRank()];
		if (H5Sget_simple_extent_dims(fSpace, dims, 0) < 0) {
			delete dims;
			throw Exception("Could not retrieve size of dimension");
		}
		size_t res = static_cast<size_t>(dims[dim]);
		delete dims;
		return res;
	}

	Dataset::Dataset(hid_t objectId, const std::string& name)
	{
		fName = name;
		fType = Object::ObjectType::Dataset;
		fObjectId = objectId;
		fSpace = H5Dget_space(fObjectId);
		if (objectId > -1) {
			updateAttributes();
		}

//		updateDataset(objectId);
	}

} /* namespace hdf5 */
