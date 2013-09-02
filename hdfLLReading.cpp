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
	/**
	 * Implementation of LowLevelData methods
	 */
	AttributeData::AttributeData(hid_t attributeId, const std::string& name):
		attributeId(attributeId), attributeName(name), type(attributeId)
	{
		using namespace std;

		// getting properties of the memory
		int rank = H5Sget_simple_extent_ndims(type.space);
		if (rank >= 0) {
			data.rank = static_cast<size_t>(rank);
		}
		delete[] data.dimensions;

		data.dimensions = new hsize_t[rank];
		if (H5Sget_simple_extent_dims(type.space, data.dimensions, 0) < 0) {
			throw Exception("AttributeData: H5Sget_simple_extent_dims returned an error");
		}

		// checking if it is a simple type or compound
		data.typeSize = 0;
		data.memorySize = 0;
		free(data.memory);
		if (type.typeClass == H5T_COMPOUND) {
			int nMembers = H5Tget_nmembers(type.dataType);
			if (nMembers < 0) {
				throw Exception("AttributeData: H5Tget_nmembers returned an error");
			}

			for(size_t iMember = 0; iMember < static_cast<size_t>(nMembers); ++iMember) {
				hid_t memberType = H5Tget_member_type(type.dataType, iMember);
				H5T_class_t tClass = H5Tget_class(memberType);
				if (tClass != H5T_COMPOUND) {
					data.typeSize += H5Tget_precision(memberType) / 8 * 2;
				}
				else {
					throw Exception("AttributeData: Found a Compound in a Compound Attribute!!!");
				}
			}
			data.memorySize = data.typeSize;
		}
		else {
			free(data.memory);
			// allocating memory for this type
			data.typeSize = data.memorySize = H5Tget_precision(type.dataType) / 8;
		}

		for (size_t i = 0; i < static_cast<size_t>(data.rank); ++i) {
			data.memorySize *= data.dimensions[i];
		}

		// ensure a memory alignment of 8Bytes
		size_t memSizeRest = 8 - (data.memorySize % 8);
		data.memorySize += (memSizeRest < 8 ? memSizeRest : 0);
//		cout << "Allocation " << data.memorySize << " Bytes / alignment rest " << memSizeRest << endl;
		if (data.memorySize < 1) {
			throw Exception("Error while determining required memory size --> got 0 Bytes!");
		}
		data.memory = malloc(data.memorySize);

		// reading data into ram
		herr_t status = H5Aread(attributeId, type.dataType, data.memory);
		if (status < 0) {
			throw hdf5::Exception("AttributeData: H5Aread returned with an error");
		}

		if (type.typeClass == H5T_COMPOUND) {
			if (rank != 0) {
				throw Exception("AttributeData: Compound types are currently only supported at rank 0");
			}
			int nMembers = H5Tget_nmembers(type.dataType);
			map<std::string, any> result;
			for(size_t iMember = 0; iMember < static_cast<size_t>(nMembers); ++iMember) {
				// retrieving current members name
				char* memberName = H5Tget_member_name(type.dataType, iMember);
				if (memberName == 0) {
					throw Exception("AttributeData:Invalid memberName=0 retrieved");
				}
				string name(memberName);
				free(memberName);

				// prepare for parsing the data
				HdfType memberType;
				memberType.attributeId = attributeId;
				memberType.dataType = H5Tget_member_type(type.dataType, iMember);
				size_t offset = H5Tget_member_offset(type.dataType, iMember);

				memberType.typeClass = H5Tget_class(memberType.dataType);
				RawData memberData;
				memberData.rank = 0;
				memberData.typeSize = H5Tget_precision(memberType.dataType) / 8;
				memberData.memory = static_cast<void*>( static_cast<uint8_t*>(data.memory) + offset );


				result[name] = parseRawValue(memberType, memberData);
			}
			parsedAttributeValue = result;
		}
		else {
			parsedAttributeValue = parseRawValue(type, data);
		}

	}

	any AttributeData::parseRawValue(HdfType aType, RawData& aData) {
		using namespace std;

		any result;

		switch(aType.typeClass) {
			case H5T_INTEGER:
			{
				H5T_sign_t sign = H5Tget_sign(aType.dataType);
				if (sign == H5T_SGN_NONE) {
					// unsigned integers
					switch (aData.typeSize * 8) {
						case 8:  result = convertToCPP<uint8_t>::get(aData); break;
						case 16: result = convertToCPP<uint16_t>::get(aData); break;
						case 32: result = convertToCPP<uint32_t>::get(aData); break;
						case 64: result = convertToCPP<uint64_t>::get(aData); break;
					}
				}
				else if (sign == H5T_SGN_2) {
					// signed integers
					switch (aData.typeSize * 8) {
						case 8:  result = convertToCPP<uint8_t>::get(aData); break;
						case 16: result = convertToCPP<uint16_t>::get(aData); break;
						case 32: result = convertToCPP<uint32_t>::get(aData); break;
						case 64: result = convertToCPP<uint64_t>::get(aData); break;
					}
				}
				else {
					throw Exception("Undefined sign of H5T_INTEGER");
				}
			}
				break;
			case H5T_FLOAT:
				if (aData.typeSize*8 == 32) {
					result = convertToCPP<float>::get(aData); break;
				}
				else if (aData.typeSize*8 == 64) {
					result = convertToCPP<double>::get(aData); break;
				}
				else {
					std::stringstream ss;
					ss << "Object::getStlType: H5T_FLOAT with unkown precision " << aData.typeSize*8 << "bits";
					throw Exception(ss);
				}
				break;
			case H5T_STRING:
			{
				H5T_cset_t charSet = H5Tget_cset(aType.dataType);
				if (charSet != 0) {
					std::stringstream ss;
					ss << "Object::getStlType: Unknown character encoding type '" << charSet << "'. Currently only US-ASCII is supported.";
					throw Exception(ss);
				}
				htri_t isVariableLen = H5Tis_variable_str(aType.dataType);
				if(isVariableLen < 0) {
					throw Exception("AttributeData: Could not retrieve information if member is of variable len");
				}

				if (!isVariableLen) {
					// the memory is already initialized so do the usual typecasting stuff
					result = convertToCPP<char*>::get(aData);
				}
				else {
					// than it is a variable string ... the hdf5 C-api takes care of the memory allocation
					RawData sMemory(aData);
					char* varString;
					herr_t status = H5Aread(aType.attributeId, aType.dataType, &varString);
					if (status < 0)
						throw Exception("AttributeData::parseRawValue: failed to read variable length string");
					sMemory.memory = static_cast<void*>(varString);

					// do the usual conversion stuff
					result = convertToCPP<char*>::get(sMemory);

					// but we need to free it
					free (sMemory.memory);
				}
			}
				break;
			default:
				throw Exception("AttributeData::parseRawValue: Unknown type");
		}

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
