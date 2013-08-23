#ifndef HDF5_DATATYPES_H_
#define HDF5_DATATYPES_H_

#include <hdf5.h>

#include <string>

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
		typedef Container PODType; // this is just for POD types correct!!!!

		static hid_t hdfType(const Container& src) { return src.hdftype(); }

		static hsize_t size(const Container& src) { return src.hdfSize(); }

		static hid_t hdfSpace(const Container& src) { return -1; }

		static hid_t isStructType() { return true; }
		static hid_t isPOD() { return false; }

		/// here you can define a handler for freeing the pod element if necessary, e.g., if you need to malloc in assignToPOD
		static void freePOD(PODType& pod) {};
		/// necessary for converting complex C++ class to pod type
		static void assignToPOD(const ElementType& in, PODType& out) {}
		/// necessary for reading non POD type data from HDF5
		static void assignFromPOD(const PODType& in, ElementType& out) {};

	};

	// basic C++ types
	template<> struct DataType<std::string> {
			typedef std::string ElementType;
			typedef char* PODType;
			static hid_t hdfType() {
				hid_t x = H5Tcopy( H5T_C_S1);
				H5Tset_size(x, H5T_VARIABLE);
				return x;
			}
			inline static hid_t isStructType() { return true; }
			inline static hid_t isPOD() { return false; }
			inline static hsize_t size() { return sizeof(PODType); }

			static void freePOD(PODType pod) { free(pod); }
			static void assignToPOD(const ElementType& in, PODType& out) {
				size_t nBytes = (in.size() + 1) * sizeof(char);
				out = static_cast<char*>(malloc(nBytes));
				if (out == 0) {
						throw Exception("Could not allocate buffer memory");
				}
//				std::cout << static_cast<void*>(out) << " / nBytes=" << nBytes << std::endl;

				for (size_t i = 0; i < in.length(); ++i) {
					out[i] = in.c_str()[i];
				}
				out[in.length()] = 0;
			};
			inline static void assignFromPOD(const PODType in, ElementType& out) { out.operator=(in); };
	};

	template<> struct DataType<char*> {
			typedef char ElementType;
			typedef char* PODType;
			static hid_t hdfType() {
				hid_t x = H5Tcopy( H5T_C_S1);
				H5Tset_size(x, H5T_VARIABLE);
				return x;
			}
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return false; }
			inline static hsize_t size() { return sizeof(char); }
			static void freePOD(PODType pod) { free(pod); }
			static void assignToPOD(const std::string& in, PODType& out) {
				out = static_cast<char*>(malloc( (in.size() + 1) * sizeof(char)));
				if (out == 0) {
					throw Exception("Could not allocate buffer memory");
				}

				for (size_t i = 0; i < in.length(); ++i) {
					out[i] = in.c_str()[i];
				}
				out[in.length()] = 0;
			}
			inline static void assignFromPOD(const PODType in, std::string& out) { out.operator=(in); };
	};

	template<> struct DataType<float> {
			typedef float ElementType;
			typedef float PODType;
			inline static hid_t hdfType() { return H5T_C_S1; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};

	template<> struct DataType<double> {
			typedef double ElementType;
			typedef double PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_DOUBLE; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<long double> {
			typedef long double ElementType;
			typedef long double PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_LDOUBLE; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};

	template<> struct DataType<int8_t>   {
			typedef int8_t ElementType;
			typedef int8_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_CHAR; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<int16_t>  {
			typedef int16_t ElementType;
			typedef int16_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_SHORT; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<int32_t>  {
			typedef int32_t ElementType;
			typedef int32_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_INT; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<int64_t>  {
			typedef int64_t ElementType;
			typedef int64_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_LLONG; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<uint8_t>  {
			typedef uint8_t ElementType;
			typedef uint8_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_UCHAR; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<uint16_t> {
			typedef uint16_t ElementType;
			typedef uint16_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_USHORT; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<uint32_t> {
			typedef uint32_t ElementType;
			typedef uint32_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_UINT; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
	};
	template<> struct DataType<uint64_t> {
			typedef uint64_t ElementType;
			typedef uint64_t PODType;
			inline static hid_t hdfType() { return H5T_NATIVE_ULLONG; }
			inline static hid_t isStructType() { return false; }
			inline static hid_t isPOD() { return true; }
			inline static hsize_t size() { return sizeof(ElementType); }
			inline static void freePOD(PODType& pod) {};
			inline static void assignToPOD(const ElementType& in, PODType& out) { out = ElementType(in); }
			inline static void assignFromPOD(const PODType& in, ElementType& out) { out = ElementType(in); };
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
