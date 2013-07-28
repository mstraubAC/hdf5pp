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
			hid_t space;
			hsize_t rank;
			hsize_t* dimensions;

			H5T_class_t typeClass;
			hid_t dataType;
			size_t precision;
			hid_t memType;
			void* memory;
			size_t memSize;

			LowLevelData() : space(0), rank(0), dimensions(0), typeClass(H5T_NO_CLASS), dataType(0), precision(0), memType(0), memory(0), memSize(0) {}
			LowLevelData(const LowLevelData& x) { operator=(x); }
			~LowLevelData();
			LowLevelData& operator=(const LowLevelData& x);

			void setSpace(hid_t inSpace);
			void setType(hid_t inType);
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
				if (data.dimensions[0] == 1) {
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
			static any get(const LowLevelData& data) {
				any result;
				if (data.dimensions[0] == 1) {
					char* tmp = static_cast<char*>(data.memory);
					result = std::string(tmp);
				}
				else if (data.rank == 1)
				{
					// store it into stl vector
					std::vector<std::string> tmp;
					tmp.reserve(data.dimensions[0]);
					const char** x = static_cast<const char**>(data.memory);
					for (size_t i = 0; i < data.dimensions[0]; ++i) {
						tmp.push_back(std::string(x[i]));
					}
					result = tmp;
				}
				else {
					throw Exception("You wanted to read a data structure from a rank higher than 4, which is not yet supported.");
				}

				return result;
			}
	};

	any llReadAttribute(hid_t attrId);

	std::string getTypeClassName(hid_t typeID);
}

#endif /* HDFLLREADING_H_ */
