#include "hdfLLReading.h"

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
#define ANYCAST_CONTAINER(CONTAINER, SRC, DST) ;

namespace std {
	std::ostream& operator<<(std::ostream& os, const boost::any& in) {
		ANYCAST(double, in, os)
		else ANYCAST(float, in, os)
		else ANYCAST(std::string, in, os)
		else ANYCAST(uint8_t, in, os)
		else ANYCAST(uint16_t, in, os)
		else ANYCAST(uint32_t, in, os)
		else ANYCAST(uint64_t, in, os)
		else ANYCAST(int8_t, in, os)
		else ANYCAST(int16_t, in, os)
		else ANYCAST(int32_t, in, os)
		else ANYCAST(int64_t, in, os)
		else ANYCAST(uint8_t, in, os)

		return os;
	}
}

namespace hdf5 {

	/**
	 * Implementation of LowLevelData methods
	 */
	LowLevelData& LowLevelData::operator=(const LowLevelData& x) {
		if (this != &x) {
			space = x.space;
			rank = x.rank;
			dimensions = x.dimensions;
			typeClass = x.typeClass;
			dataType = x.dataType;
			precision = x.precision;
			memType = x.memType;
			memory = x.memory;
			memSize = x.memSize;
		}

		return *this;
	}
	void LowLevelData::setSpace(hid_t inSpace) {
		space = inSpace;
		int ret = H5Sget_simple_extent_ndims(inSpace);
		if (ret >= 0) {
			rank = static_cast<hsize_t>(ret);
		}
		delete dimensions;
		dimensions = new hsize_t[rank];
		H5Sget_simple_extent_dims(space, dimensions, 0);
	}
	void LowLevelData::setType(hid_t inType) {
		dataType = inType;
		precision = H5Tget_precision(dataType);
		typeClass = H5Tget_class(dataType);
	}
	void LowLevelData::initMemory() {
		// calculate and allocate memory to store data
		free(memory);
		memSize = precision / 8;
		for (size_t i = 0; i < rank; ++i)
			memSize *= dimensions[i];
		memory = malloc(memSize);
	}
	LowLevelData::~LowLevelData() {
		free(memory);
		delete dimensions;
	}

	/**
	 * Implementation of other stuff
	 */
	any llReadAttribute(hid_t attrId) {
		LowLevelData data;

		// get extensions of the data array
		data.setSpace(H5Aget_space(attrId));


		// get hdf5 type of data element
		data.setType(H5Aget_type(attrId));

		// init memory
		data.initMemory();

		// read attribute into allocated memory
		herr_t res = H5Aread(attrId, data.dataType, data.memory);
		if (res < 0) {
			throw 0;
		}

		/*
		 *  convert it from a C POD to a C++ type
		 */
		any result;
		switch(data.typeClass) {
			case H5T_INTEGER:
			{
				H5T_sign_t sign = H5Tget_sign(data.dataType);
				if (sign == H5T_SGN_NONE) {
					// unsigned integers
					switch (data.precision) {
						case 8:  result = convertToCPP<int8_t>(data); break;
						case 16: result = convertToCPP<int16_t>(data); break;
						case 32: result = convertToCPP<int32_t>(data); break;
						case 64: result = convertToCPP<int64_t>(data); break;
					}
				}
				else if (sign == H5T_SGN_2) {
					// signed integers
					switch (data.precision) {
						case 8:  result = convertToCPP<uint8_t>(data); break;
						case 16: result = convertToCPP<uint16_t>(data); break;
						case 32: result = convertToCPP<uint32_t>(data); break;
						case 64: result = convertToCPP<uint64_t>(data); break;
					}
				}
				else {
					throw Exception("Undefined sign of H5T_INTEGER");
				}
			}
				break;
			case H5T_FLOAT:
			{
				if (data.precision == 32) {
					result = convertToCPP<float>(data); break;
				}
				else if (data.precision == 64) {
					result = convertToCPP<double>(data); break;
				}
				else {
					std::stringstream ss;
					ss << "Object::getStlType: H5T_FLOAT with unkown precision " << data.precision << "bits";
					throw Exception(ss);
				}
			}
				break;
			case H5T_STRING:
//				result = convertToCPP<std::string>(data); break;
//				H5T_cset_t charSet = H5Tget_cset(hdfTypeId);
//				if (charSet != 0) {
//					std::stringstream ss;
//					ss << "Object::getStlType: Unknown character encoding type '" << charSet << "'. Currently only US-ASCII is supported.";
//					throw Exception(ss);
//				}
//				size = H5Tget_precision(hdfTypeId) * sizeof(char);
//				for (size_t i = 0; i < rank; ++i) size *= dimensions[i];
//				data = malloc(size);
				break;
			default:
				throw Exception("Object::getStorage: Unimplemented HDF5 datatype");
		}

		return result;

	}
}
