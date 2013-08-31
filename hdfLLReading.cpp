#include "hdfLLReading.h"
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include "Exception.h"
#include <boost/preprocessor/repetition/repeat.hpp>

#define BASIC_TYPE(I) BASIC_TYPE ## I

#define BASIC_TYPE0 bool
#define BASIC_TYPE1 int8_t
#define BASIC_TYPE2 uint8_t
#define BASIC_TYPE3 int16_t
#define BASIC_TYPE4 uint16_t
#define BASIC_TYPE5 int32_t
#define BASIC_TYPE6 uint32_t
#define BASIC_TYPE7 int64_t
#define BASIC_TYPE8 uint64_t
#define BASIC_TYPE9 float
#define BASIC_TYPE10 double
#define BASIC_TYPE11 long double
#define BASIC_TYPE12 std::string

#define BASIC_TYPE_CNT 13

#define ANYCAST_BASIC(TYPE, SRC, DST) if (SRC.type() == typeid(TYPE)) DST << boost::any_cast<TYPE>(SRC);


namespace std {
	// this is just a convenience stream operator to pipe c++ default types
	std::ostream& operator<<(std::ostream& os, const boost::any& in) {
		ANYCAST_BASIC(double, in, os)
		else ANYCAST_BASIC(float, in, os)
		else ANYCAST_BASIC(std::string, in, os)
		else ANYCAST_BASIC(uint8_t, in, os)
		else ANYCAST_BASIC(uint16_t, in, os)
		else ANYCAST_BASIC(uint32_t, in, os)
		else ANYCAST_BASIC(uint64_t, in, os)
		else ANYCAST_BASIC(int8_t, in, os)
		else ANYCAST_BASIC(int16_t, in, os)
		else ANYCAST_BASIC(int32_t, in, os)
		else ANYCAST_BASIC(int64_t, in, os)
		else ANYCAST_BASIC(uint8_t, in, os)
		return os;
	}
}

namespace hdf5 {
	LowLevelData::LowLevelData(hid_t attributeId) :
			attrId(attributeId),
			space(0), rank(0), dimensions(0),
			varString(-1),
			typeClass(H5T_NO_CLASS), dataType(0),
			precision(0), memType(0), memory(0), memSize(0)
	{
		setSpace();
		setType();
	}

	/**
	 * Implementation of LowLevelData methods
	 */
	LowLevelData& LowLevelData::operator=(const LowLevelData& x) {
		if (this != &x) {
			attrId = x.attrId;
			space = x.space;
			rank = x.rank;
			dimensions = x.dimensions;
			varString = x.varString;
			typeClass = x.typeClass;
			dataType = x.dataType;
			precision = x.precision;
			memType = x.memType;
			memory = x.memory;
			memSize = x.memSize;
		}

		return *this;
	}
	void LowLevelData::setSpace() {
		space = H5Aget_space(attrId);
		int ret = H5Sget_simple_extent_ndims(space);
		if (ret >= 0) {
			rank = static_cast<hsize_t>(ret);
		}
		delete dimensions;
		dimensions = new hsize_t[rank];
		H5Sget_simple_extent_dims(space, dimensions, 0);
	}
	void LowLevelData::setType() {
		using namespace std;

		dataType = H5Aget_type(attrId);
		typeClass = H5Tget_class(dataType);

		if (typeClass == H5T_COMPOUND) {
			precision = 0;
		}
		else {
			precision = H5Tget_precision(dataType);
		}
	}
	void LowLevelData::initMemory() {
		// calculate and allocate memory to store data
		free(memory);
		if (typeClass == H5T_COMPOUND) {
			memSize = 0;
			int nMembers = H5Tget_nmembers(dataType);
			for (size_t iMember = 0; iMember < static_cast<size_t>(nMembers); ++iMember) {
				hid_t memberType = H5Tget_member_type(dataType, iMember);
				H5T_class_t tClass = H5Tget_class(memberType);
				if (tClass == H5T_COMPOUND) {
					//
				}
				else {
					memSize += H5Tget_precision(memberType) / 8 * 2;
				}
			}
		}
		else if (typeClass == H5T_STRING) {
			htri_t varStr = H5Tis_variable_str(dataType);
			if (varStr < 0) {
				throw Exception("H5Tis_variable_str failed");
			}

			if (!varStr) {
				memSize = H5Tget_size(dataType) +1; // for the null termination
			}
			else {
				memSize = 42; // dummy
			}
		}
		else {
			memSize = precision / 8;
		}

		// handle the increased storage requirements for array structured data
		for (size_t i = 0; i < rank; ++i)
			memSize *= dimensions[i];

		memory = calloc(1, memSize);

		// read attribute into allocated memory
		herr_t res = H5Aread(attrId, dataType, memory);
		if (res < 0) {
			throw hdf5::Exception("Could not read attribute");
		}
	}
	LowLevelData::~LowLevelData() {
		free(memory);
		delete dimensions;
	}

	/**
	 * Implementation of other stuff
	 */
	any llReadAttribute(hid_t attrId) {
		using namespace std;
		// initialize struct handling all memory management stuff
		LowLevelData data(attrId);

		/*
		 *  convert it from a C POD to a C++ type
		 */
		any result;
		switch(data.typeClass) {
			case H5T_INTEGER:
			{
				// allocate memory and read attribute
				data.initMemory();

				H5T_sign_t sign = H5Tget_sign(data.dataType);
				if (sign == H5T_SGN_NONE) {
					// unsigned integers
					switch (data.precision) {
						case 8:  result = convertToCPP<uint8_t>::get(data); break;
						case 16: result = convertToCPP<uint16_t>::get(data); break;
						case 32: result = convertToCPP<uint32_t>::get(data); break;
						case 64: result = convertToCPP<uint64_t>::get(data); break;
					}
				}
				else if (sign == H5T_SGN_2) {
					// signed integers
					switch (data.precision) {
						case 8:  result = convertToCPP<int8_t>::get(data); break;
						case 16: result = convertToCPP<int16_t>::get(data); break;
						case 32: result = convertToCPP<int32_t>::get(data); break;
						case 64: result = convertToCPP<int64_t>::get(data); break;
					}
				}
				else {
					throw Exception("Undefined sign of H5T_INTEGER");
				}
			}
				break;
			case H5T_FLOAT:
			{
				// allocate memory and read attribute
				data.initMemory();

				if (data.precision == 32) {
					result = convertToCPP<float>::get(data); break;
				}
				else if (data.precision == 64) {
					result = convertToCPP<double>::get(data); break;
				}
				else {
					std::stringstream ss;
					ss << "Object::getStlType: H5T_FLOAT with unkown precision " << data.precision << "bits";
					throw Exception(ss);
				}
			}
				break;
			case H5T_STRING:
			{
				H5T_cset_t charSet = H5Tget_cset(data.dataType);
				if (charSet != 0) {
					std::stringstream ss;
					ss << "Object::getStlType: Unknown character encoding type '" << charSet << "'. Currently only US-ASCII is supported.";
					throw Exception(ss);
				}
				result = convertToCPP<char*>::get(data);
			}
				break;
			case H5T_COMPOUND:
				// allocate memory and read attribute
				data.initMemory();

				result = llReadCompound(data);
				break;
			default:
				throw Exception("Object::getStorage: Unimplemented HDF5 datatype");
		}

		return result;

	}

	any llReadCompound(const LowLevelData& data) {
		using namespace std;

		int nMembers = H5Tget_nmembers(data.dataType);
		if (nMembers < 0) {
			return std::string("Could not read compound");
		}

		map<std::string, any> result;
		for (size_t iMember = 0; iMember < static_cast<size_t>(nMembers); ++iMember) {
			char* memberName = H5Tget_member_name(data.dataType, iMember);
			string name(memberName);
			free(memberName);

			hid_t memberType = H5Tget_member_type(data.dataType, iMember);
			size_t tSize = H5Tget_precision(memberType);
			size_t offset = H5Tget_member_offset(data.dataType, iMember);
			// start address of this member in memory
			void* mem = static_cast<void*>(static_cast<uint8_t*>(data.memory) + offset);

			switch(H5Tget_class(memberType)) {
				case H5T_INTEGER:
				{
					H5T_sign_t sign = H5Tget_sign(memberType);
					if (sign == H5T_SGN_NONE) {
						// unsigned integers
						switch (tSize) {
							case 8:  result[name] = *static_cast<uint8_t*>(mem); break;
							case 16: result[name] = *static_cast<uint16_t*>(mem); break;
							case 32: result[name] = *static_cast<uint32_t*>(mem); break;
							case 64: result[name] = *static_cast<uint64_t*>(mem); break;
						}
					}
					else if (sign == H5T_SGN_2) {
						// signed integers
						switch (tSize) {
							case 8:  result[name] = *static_cast<int8_t*>(mem); break;
							case 16: result[name] = *static_cast<int16_t*>(mem); break;
							case 32: result[name] = *static_cast<int32_t*>(mem); break;
							case 64: result[name] = *static_cast<int64_t*>(mem); break;
						}
					}
					else {
						throw Exception("Undefined sign of H5T_INTEGER");
					}
				}
					break;
				case H5T_FLOAT:
				{
					if (tSize == 32) {
						result[name] = *static_cast<float*>(mem); break;
					}
					else if (tSize == 64) {
						result[name] = *static_cast<double*>(mem); break;
					}
					else {
						std::stringstream ss;
						ss << "Object::getStlType: H5T_FLOAT with unkown precision " << data.precision << "bits";
						throw Exception(ss);
					}
				}
					break;
				case H5T_STRING:
//				{
//					H5T_cset_t charSet = H5Tget_cset(data.dataType);
//					if (charSet != 0) {
//						std::stringstream ss;
//						ss << "Object::getStlType: Unknown character encoding type '" << charSet << "'. Currently only US-ASCII is supported.";
//						throw Exception(ss);
//					}
//					result = convertToCPP<char*>::get(data);
//				}
					break;
				default:
					throw hdf5::Exception("llReadCompound not implemented type");
			}

		}

//		for (map<std::string, any>::const_iterator it = result.begin(); it != result.end(); ++it) {
//			cout << " " << it->first << " ---> " << it->second << endl;
//		}
		return result;
	}

	std::string getTypeClassName(hid_t typeID)
	{
		switch(H5Tget_class(typeID)) {
			case H5T_INTEGER: return "H5T_INTEGER";
			case H5T_FLOAT: return "H5T_FLOAT";
			case H5T_STRING: return "H5T_STRING";
			case H5T_TIME: return "H5T_TIME";
			case H5T_BITFIELD: return "H5T_BITFIELD";
			case H5T_OPAQUE: return "H5T_OPAQUE";
			case H5T_COMPOUND:
			{
				std::stringstream ss;
				ss << "H5T_COMPOUND";
				int nMembers = H5Tget_nmembers(typeID);
				if (nMembers < 0) {
					ss << ": Could not retrieve members";
					return ss.str();
				}
				for (size_t iMember = 0; iMember < static_cast<size_t>(nMembers); ++iMember) {
					char* memberName = H5Tget_member_name(typeID, iMember);
					hid_t memberType = H5Tget_member_type(typeID, iMember);
					ss << std::endl;
					ss << "    * " << memberName << ": " << getTypeClassName(memberType);
					free(memberName);
				}
				ss << std::endl;
				return ss.str();
			}
				break;
			case H5T_REFERENCE: return "H5T_REFERENCE";
			case H5T_ENUM: return "H5T_ENUM";
			case H5T_VLEN: return "H5T_VLEN";
			case H5T_ARRAY: return "H5T_ARRAY";
			default:
				return "UNKNOWN";
		}
	}

}
