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

	/**
	 * Container holding raw data including information about its geometry
	 */
	struct RawData {
			size_t rank;
			hsize_t* dimensions;
			size_t typeSize;
			size_t memorySize;
			void* memory;

			RawData(): rank(0), dimensions(0), typeSize(0), memorySize(0), memory(0) {}
			RawData(const RawData& z) { operator=(z); }
			virtual ~RawData() {};

			/**
			 * Be carefult this copy constructor copies the pointers not the memory content!!!
			 *
			 * @param z
			 * @return
			 */
			RawData& operator=(const RawData& z) {
				if (&z != this) {
					rank = z.rank;
					dimensions = z.dimensions;
					typeSize = z.typeSize;
					memorySize = z.memorySize;
					memory = z.memory;
				}
				return *this;
			}
	};

	/**
	 * Holding information about an HDF5 type
	 */
	struct HdfType {
			hid_t space;

			H5T_class_t typeClass;
			hid_t dataType;
			hid_t attributeId;

			HdfType(): space(-1), typeClass(H5T_NO_CLASS), dataType(-1), attributeId(-1) {}
			HdfType(hid_t attributeId) {
				this->attributeId = attributeId;
				space = H5Aget_space(attributeId);
				dataType = H5Aget_type(attributeId);
				typeClass = H5Tget_class(dataType);
			}
			HdfType(const HdfType& x) { operator=(x); }
			virtual ~HdfType() {};
			HdfType& operator=(const HdfType& x) {
				if (&x != this) {
					attributeId = x.attributeId;
					space = x.space;
					typeClass = x.typeClass;
					dataType = x.dataType;
				}
				return *this;
			}
	};

	/**
	 *  Single values shall be converted its closest type.
	 *  Arrays are converted into boost::multi_array
	 *
	 *  We are using ugly code to allow matrized upto rank 4.
	 */
	template<typename T> struct convertToCPP {
			static any get(const RawData& data) {
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
//				else if (data.rank == 2) {
//					// store it into boost::multi_array
//					result = boost::multi_array<T, 2>();
//				}
//				else if (data.rank == 3) {
//					// store it into boost::multi_array
//					result = boost::multi_array<T, 3>();
//				}
//				else if (data.rank == 4) {
//					// store it into boost::multi_array
//					result = boost::multi_array<T, 4>();
//				}
				else {
					throw Exception("You wanted to read a data structure from a rank higher than 1, which is not yet supported.");
				}
				return result;
			}
	};

	template<> struct convertToCPP<char*> {
			static any get(RawData& data) {
				using namespace std;
				any result;

				if (data.rank == 0 || (data.rank == 1 && data.dimensions[0] == 1)) {
					char* tmp = static_cast<char*>(data.memory);
					result = std::string(tmp);
				}
				else {
					cerr << "Error: string of rank=" << data.rank << " other than 0 or rank=1 & dim[0]=1!!!!!" << endl;
				}
				return result;
			}
	};

	/**
	 * Attribute data converter
	 */
	struct AttributeData {
			hid_t attributeId;
			std::string attributeName;

			HdfType type;
			RawData data;

			any parsedAttributeValue;

			AttributeData(): attributeId(-1), attributeName("") {}
			AttributeData(const AttributeData& x) { operator=(x); }
			AttributeData(hid_t attributeId, const std::string& name);
			virtual ~AttributeData() { free(data.memory); delete[] data.dimensions; };

			static any parseRawValue(HdfType aType, RawData& aData);

			AttributeData& operator=(const AttributeData& x) {
				if (&x != this) {

				}
				return *this;
			}
	};

	any llReadAttribute(hid_t attrId) {
		AttributeData attr(attrId, "No Name");
		return attr.parsedAttributeValue;
	}

	std::string getTypeClassName(hid_t typeID);
}

#endif /* HDFLLREADING_H_ */
