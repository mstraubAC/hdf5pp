#ifndef HDF5_DATATYPES_H_
#define HDF5_DATATYPES_H_

#include <hdf5.h>

namespace hdf5 {
	/**
	 * Use this base class to get best performance
	 */
	class DatasetStore {
		public:
			DatasetStore() {}
			DatasetStore(const DatasetStore&) {}
			virtual ~DatasetStore() {}

			virtual hid_t hdfType() const = 0;
			virtual hsize_t hdfSize() const = 0;
			virtual bool hdfWriteAccess() = 0;
			virtual const void* hdfReadAccess() const = 0;
	};

	/**
	 * Via templates nearly every container should be possible to access
	 * the non specialized type is handling the use of the DatasetStore class
	 */
	template<typename Container> struct DataType {
		typedef Container ElementType;

		static hid_t hdfType(const Container& src) { return src.hdftype(); }

		static hsize_t size(const Container& src) { return src.hdfSize(); }

		static hid_t hdfSpace(const Container& src) { return -1; }

		static hid_t isStructType() { return true; }
		static hid_t isPOD() { return false; }

	};

	// basic C++ types
	template<> struct DataType<float> {
			typedef float ElementType;
			typedef float PODType;
			static hid_t hdfType() { return H5T_NATIVE_FLOAT; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(float); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<double> {
			typedef double ElementType;
			typedef double PODType;
			static hid_t hdfType() { return H5T_NATIVE_DOUBLE; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(double); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<long double> {
			typedef long double ElementType;
			typedef long double PODType;
			static hid_t hdfType() { return H5T_NATIVE_LDOUBLE; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(long double); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};

	template<> struct DataType<int8_t>   {
			typedef int8_t ElementType;
			typedef int8_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_CHAR; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(int8_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<int16_t>  {
			typedef int16_t ElementType;
			typedef int16_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_SHORT; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(int16_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<int32_t>  {
			typedef int32_t ElementType;
			typedef int32_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_INT; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(int32_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<int64_t>  {
			typedef int64_t ElementType;
			typedef int64_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_LLONG; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(int64_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<uint8_t>  {
			typedef uint8_t ElementType;
			typedef uint8_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_UCHAR; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(uint8_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<uint16_t> {
			typedef uint16_t ElementType;
			typedef uint16_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_USHORT; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(uint16_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<uint32_t> {
			typedef uint32_t ElementType;
			typedef uint32_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_UINT; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(uint32_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};
	template<> struct DataType<uint64_t> {
			typedef uint64_t ElementType;
			typedef uint64_t PODType;
			static hid_t hdfType() { return H5T_NATIVE_ULLONG; }
			static hid_t isStructType() { return false; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(uint64_t); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
	};

//	//
//	template<> struct DataType<Vector> {
//			typedef Vector ElementType;
//			typedef Vector::POD PODType;
//
//			static hid_t hdfType() {
//				hid_t t = H5Tcreate(H5T_COMPOUND, size());
//				H5Tinsert(t, "X", HOFFSET(PODType, X), H5T_NATIVE_DOUBLE);
//				H5Tinsert(t, "Y", HOFFSET(PODType, Y), H5T_NATIVE_DOUBLE);
//				H5Tinsert(t, "Z", HOFFSET(PODType, Z), H5T_NATIVE_DOUBLE);
//				return t;
//			}
//			static hid_t isStructType() { return true; }
//			static hid_t isPOD() { return false; }
//			static hsize_t size() { return sizeof(PODType); }
//			static void assignToPOD(const ElementType& in, PODType& out) {
//				out.X = in.getX();
//				out.Y = in.getY();
//				out.Z = in.getZ();
//			}
//	};

};

#endif