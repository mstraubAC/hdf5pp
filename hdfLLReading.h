/*
 * hdfLLReading.h
 *
 * Here we handle all the pointer stuff
 *
 *  Created on: Jul 22, 2013
 *      Author: marcel
 */

#ifndef HDF5_HDFLLREADING_H_
#define HDF5_HDFLLREADING_H_

#include "Exception.h"
#include <hdf5.h>
#include <string>
#include <iostream>
#include <boost/any.hpp>
#include <boost/spirit/home/support/detail/hold_any.hpp>
#include <boost/multi_array.hpp>

namespace std {
	std::ostream& operator<<(std::ostream& os, const boost::any& in);
}

namespace hdf5 {
//	typedef boost::spirit::hold_any any;
	typedef boost::any any;

	struct LowLevelData {
			hid_t attrId;
			hid_t space;
			hsize_t rank;
			hsize_t* dimensions;

			htri_t varString;
			H5T_class_t typeClass;
			hid_t dataType;
			size_t precision;
			hid_t memType;
			void* memory;
			size_t memSize;

			LowLevelData() :
				attrId(0), space(0), rank(0), dimensions(0),
				varString(-1), typeClass(H5T_NO_CLASS),
				dataType(0), precision(0), memType(0),
				memory(0), memSize(0) {}
			LowLevelData(hid_t attributeId);
			LowLevelData(const LowLevelData& x) { operator=(x); }
			~LowLevelData();
			LowLevelData& operator=(const LowLevelData& x);

			void setSpace();
			void setType();
			void initMemory();

	};

	/**
	 *  Single values shall be converted its closest type.
	 *  Arrays are converted into boost::multi_array
	 *
	 *  We are using ugly code to allow matrized upto rank 4.
	 */

	template<typename T> struct convertToCPP {
			static any get(const LowLevelData& data) {
				any result;
				if (data.rank == 0) {
					T* x = static_cast<T*>(data.memory);
					result = T(*x);
				}
				else if (data.rank == 1)
				{
					// store it into stl vector
					std::vector<T> tmp;
					tmp.reserve(data.dimensions[0]);
					const T* x = static_cast<const T*>(data.memory);
					for (size_t i = 0; i < data.dimensions[0]; ++i) {
						tmp.push_back(T(x[i]));
					}
					result = tmp;
				}
				else if (data.rank == 2) {
					// store it into boost::multi_array
					result = boost::multi_array<T, 2>();
				}
				else if (data.rank == 3) {
					// store it into boost::multi_array
					result = boost::multi_array<T, 3>();
				}
				else if (data.rank == 4) {
					// store it into boost::multi_array
					result = boost::multi_array<T, 4>();
				}
				else {
					throw Exception("You wanted to read a data structure from a rank higher than 4, which is not yet supported.");
				}
				return result;
			}
	};

	template<> struct convertToCPP<char*> {
			static any get(LowLevelData& data) {
				using namespace std;
				any result;

				if (data.rank == 0) {
					if (!data.varString) {
						// allocate memory and read attribute
						data.initMemory();
						char* tmp = static_cast<char*>(data.memory);
						result = std::string(tmp);
					}
					else {
						char* cString;
						herr_t status = H5Aread(data.attrId, data.dataType, &cString);
						if (status < 0)
							throw Exception("convertToCPP<char*> failed to read variable length string");

						result = std::string(cString);
						free(cString);
					}
				}
				else {
					cout << "Error: string not from rank 0!!!!!" << endl;
				}
				cout << "--> string=" << result << endl;
				return result;
			}
	};

	any llReadCompound(const LowLevelData& data);

	any llReadAttribute(hid_t attrId);

	std::string getTypeClassName(hid_t typeID);
}

#endif /* HDFLLREADING_H_ */
