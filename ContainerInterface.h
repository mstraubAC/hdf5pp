#ifndef HDF5_CONTAINERINTERFACE_H_
#define HDF5_CONTAINERINTERFACE_H_

#include <hdf5.h>
#include "Exception.h"

#include <vector>
#include <list>
#include <map>

namespace hdf5 {
	template<typename T> struct ContainerInterface {
			typedef T Container;

			/**
			 * Get the internal HDF5 type of the elements stored in the container
			 * @return HDF5 type identifier of element type
			 */
			static hid_t hdfElementType() { return -1; }
			/**
			 * Creates an HDF5 dataspace fitting the container.
			 * @param src The container we want to convert to/from a HDF5 dataset
			 * @returns HDF5 dataspace identifier fitting the container
			 */
			static hid_t hdfSpace(const Container& src) { return -1; }

			/**
			 * Performs the writing of the src container to the defined dataset
			 * @param src Container to write
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void write(const Container& src, hid_t dataSet, hid_t dataSpace);

			/**
			 * Reads the data from the dataSet into the dst container
			 * @param src Container to store the data from file
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void read(Container& dst, hid_t dataSet, hid_t dataSpace);
	};

	/*
	 * Implementation for STL Containers
	 */
	// std::vector<ElementType>
	template<typename ElementType, typename Allocator> struct ContainerInterface<std::vector<ElementType, Allocator> > {
			typedef typename std::vector<ElementType, Allocator> Container;

			static hid_t hdfElementType() { return DataType<ElementType>::hdfType(); }
			static hid_t hdfSpace(const Container& src) {
				hsize_t dims[] = { src.size(), };
//				hsize_t maxDims[] = { H5S_UNLIMITED, };

				return H5Screate_simple(1, dims, 0);
			}

			/**
			 * Check if the type of the provided dataset and the container matches
			 * @param src
			 * @param dataSet
			 * @param hdfMemLayout
			 * @return
			 */
			static bool checkCompatibility(const Container& src, hid_t dataSet, hid_t hdfMemLayout) {
				const size_t NumDims = 1;
				// check if type of the target dataset and the one stored in this container matches
				htri_t type = H5Tequal(DataType<ElementType>::hdfType(), H5Dget_type(dataSet));
				if (type < 1) {
					throw Exception("HDF5 and Container element type declaration does not match");
				}

				// check if rank matches
				int rank = H5Sget_simple_extent_ndims(hdfMemLayout);
				if (rank < 0) {
					throw Exception("Could not get dimensionality of dataset");
				}
				if ( (size_t)rank != NumDims) {
					throw Exception("Rank of source and destination does not match! STL vectors are just of rank 1.");
				}

				// check dimensions
				hsize_t* dims = new hsize_t[rank];
				if (H5Sget_simple_extent_dims(hdfMemLayout, dims, 0) < 0) {
					delete dims;
					throw Exception("Could not retrieve size of dimensions");
				}

				bool sizeFit = dims[0] == src.size();

				// freeing memory
				free(dims);

				if (!sizeFit) {
					throw Exception("Dimensions between dataset and provided container does not match");
				}

				return true;
			}

			/**
			 * Performs the writing of the src container to the defined dataset
			 * @param src Container to write
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void write(const Container& src, hid_t dataSet, hid_t dataSpace) {
				std::cout << "hdf5::ContainerInterface< std::vector<..> >::write()" << std::endl;

				// checking compatibility of hdf5 target and the c++ src object
				if (!checkCompatibility(src, dataSet, dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::vector<..> >::write(): Type compatibility check failed");
				}

				// check type of elements stored in container
				if (DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD()) {
					// ok that's no very efficient but we have to copy the elements twice
					// firstly we have to get the POD equivalent of the source type and fill it
					// before writing data out
					typedef typename DataType<ElementType>::PODType POD;
					typedef typename std::vector<POD> DstContainer;

					size_t nElements = src.size();
					DstContainer dst;
					dst.resize(nElements);

					for (size_t i = 0; i < nElements; ++i) {
						DataType<ElementType>::assignToPOD(src[i], dst[i]);
					}

					H5Dwrite(dataSet, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, dst.data());

//					for (size_t i = 0; i < nElements; ++i) {
//						DataType<ElementType>::freePOD(dst[i]);
//					}
				}
				else {
					// this allows simple write
					H5Dwrite(dataSet, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, src.data());
				}

			}

			/**
			 * Reads the data from the dataSet into the dst container
			 * @param src Container to store the data from file
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void read(Container& dst, hid_t dataSet, hid_t dataSpace) {
				const size_t NumDims = 1;
				std::cout << "hdf5::ContainerInterface< std::vector<..> >::read()" << std::endl;

				if (NumDims != H5Sget_simple_extent_ndims(dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::vector<..> >::read(): Dimensions of HDF5 and target container does not match");
				}

				// resize vector to be fitting for data from file
				hsize_t* dims = new hsize_t[NumDims];
				H5Sget_simple_extent_dims(dataSpace, dims, 0);
				dst.resize(dims[0]);
				delete dims;

				// checking compatibility of hdf5 target and the c++ src object
				if (!checkCompatibility(dst, dataSet, dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::vector<..> >::read(): Type compatibility check failed");
				}


				// check type of elements stored in container
				// data can only be read to a POD structure therefore with use this...
				typedef typename DataType<ElementType>::PODType POD;

				POD* rawData = (POD*) malloc( DataType<ElementType>::size() * dst.size());
				herr_t status = H5Dread(dataSet, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, rawData);
				if (status < 0) {
					free(rawData);
					throw Exception("hdf5::ContainerInterface< std::vector<..> >::read(): Error while reading data from file");
				}

				for (size_t i = 0; i < dst.size(); ++i) {
					if (DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD()) {
						// .. it target is not a POD we need to translate ..
						DataType<ElementType>::assignFromPOD(rawData[i], dst[i]);
					}
					else {
						// .. otherwise we can just copy it from the raw memory
						//TODO: this is very dangerous if ElementType is a pointer!!!
						dst[i] = rawData[i];
					}
				}

				free(rawData);
			}
	};

	// std::list<ElementType>
	template<typename ElementType, typename Allocator> struct ContainerInterface<std::list<ElementType, Allocator> > {
			typedef typename std::list<ElementType, Allocator> Container;

			static hid_t hdfElementType() { return DataType<ElementType>::hdfType(); }
			static hid_t hdfSpace(const Container& src) {
				hsize_t dims[] = { src.size(), };
//				hsize_t maxDims[] = { H5S_UNLIMITED, };

				return H5Screate_simple(1, dims, 0);
			}

			/**
			 * Check if the type of the provided dataset and the container matches
			 * @param src
			 * @param dataSet
			 * @param hdfMemLayout
			 * @return
			 */
			static bool checkCompatibility(const Container& src, hid_t dataSet, hid_t hdfMemLayout) {
				const size_t NumDims = 1;
				// check if type of the target dataset and the one stored in this container matches
				htri_t type = H5Tequal(DataType<ElementType>::hdfType(), H5Dget_type(dataSet));
				if (type < 1) {
					throw Exception("HDF5 and Container element type declaration does not match");
				}

				// check if rank matches
				int rank = H5Sget_simple_extent_ndims(hdfMemLayout);
				if (rank < 0) {
					throw Exception("Could not get dimensionality of dataset");
				}
				if ( (size_t)rank != NumDims) {
					throw Exception("Rank of source and destination does not match! STL vectors are just of rank 1.");
				}

				// check dimensions
				hsize_t* dims = new hsize_t[rank];
				if (H5Sget_simple_extent_dims(hdfMemLayout, dims, 0) < 0) {
					delete dims;
					throw Exception("Could not retrieve size of dimensions");
				}

				bool sizeFit = dims[0] == src.size();

				// freeing memory
				free(dims);

				if (!sizeFit) {
					throw Exception("Dimensions between dataset and provided container does not match");
				}

				return true;
			}

			/**
			 * Performs the writing of the src container to the defined dataset
			 * @param src Container to write
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void write(const Container& src, hid_t dataSet, hid_t dataSpace) {
				std::cout << "hdf5::ContainerInterface< std::list<..> >::write()" << std::endl;

				// checking compatibility of hdf5 target and the c++ src object
				if (!checkCompatibility(src, dataSet, dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::list<..> >::write(): Type compatibility check failed");
				}


				// check type of elements stored in container
				bool complexType = DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD();
				typedef typename DataType<ElementType>::PODType POD;
				size_t nElements = src.size();
				size_t item = 0;
				POD* tmp = (POD*) malloc( DataType<ElementType>::size() * nElements);

				for (typename Container::const_iterator it = src.begin(); it != src.end(); ++it) {
					if (complexType) {
						DataType<ElementType>::assignToPOD(*it, tmp[item]);
					}
					else {
						tmp[item] = *it;
					}

					++item;
				}

				H5Dwrite(dataSet, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, tmp);

//				for (size_t i = 0; i < item; ++i) {
//					DataType<POD>::freePOD(tmp[i]);
//				}
				free(tmp);
			}

			/**
			 * Reads the data from the dataSet into the dst container
			 * @param src Container to store the data from file
			 * @param dataSet HDF5 identifier for the target dataset
			 * @param dataSpace HDF5 identifier fitting the container
			 */
			static void read(Container& dst, hid_t dataSet, hid_t dataSpace) {
				const size_t NumDims = 1;
				std::cout << "hdf5::ContainerInterface< std::list<..> >::read()" << std::endl;

				if (NumDims != H5Sget_simple_extent_ndims(dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::list<..> >::read(): Dimensions of HDF5 and target container does not match");
				}

				// resize vector to be fitting for data from file
				hsize_t* dims = new hsize_t[NumDims];
				H5Sget_simple_extent_dims(dataSpace, dims, 0);
				size_t nElements = dst.resize(dims[0]);
				delete dims;


				// check type of elements stored in container
				// data can only be read to a POD structure therefore with use this...
				typedef typename DataType<ElementType>::PODType POD;

				POD* rawData = (POD*) malloc( DataType<ElementType>::size() * dst.size());
				herr_t status = H5Dread(dataSet, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, rawData);
				if (status < 0) {
					free(rawData);
					throw Exception("hdf5::ContainerInterface< std::vector<..> >::read(): Error while reading data from file");
				}

				for (size_t i = 0; i < dst.size(); ++i) {
					if (DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD()) {
						// .. it target is not a POD we need to translate ..
						ElementType tmp;
						DataType<ElementType>::assignFromPOD(rawData[i], tmp);
					}
					else {
						// .. otherwise we can just copy it from the raw memory
						//TODO: this is very dangerous if ElementType is a pointer!!!
						dst.push_back(rawData[i]);
					}
				}

				free(rawData);
			}
	};

	// Everything we need for converting std::map<Key,Value> back and forth...
	template<typename Key, typename Value> struct POD_STL_MAP {
			Key k;
			Value v;
	};

	template<typename Key, typename Value> struct DataType< POD_STL_MAP<Key, Value> > {
			typedef POD_STL_MAP<Key, Value> ElementType;
			typedef ElementType PODType;

			static hid_t hdfType() {
				hid_t t = H5Tcreate(H5T_COMPOUND, size());
				H5Tinsert(t, "Key", HOFFSET(PODType, k), DataType<Key>::hdfType());
				H5Tinsert(t, "Value", HOFFSET(PODType, v), DataType<Value>::hdfType());
				return t;
			}
			static hid_t isStructType() { return true; }
			static hid_t isPOD() { return true; }
			static hsize_t size() { return sizeof(ElementType); }
			static void assignToPOD(const ElementType& in, PODType& out) {  }
			static void assignFromPOD(const PODType& in, ElementType& out) {};
	};

	template<typename Key, typename Value, typename Compare, typename Allocator> struct ContainerInterface< std::map<Key, Value, Compare, Allocator> > {
			typedef typename std::map<Key, Value, Compare, Allocator> Container;
//			typedef POD_STL_MAP<Key, Value> Element;

			// POD Types for storage
			typedef typename DataType<Key>::PODType KeyPOD;
			typedef typename DataType<Value>::PODType ValuePOD;
			typedef POD_STL_MAP<KeyPOD, ValuePOD> ElementPOD;

			static hid_t hdfElementType() { return DataType< ElementPOD >::hdfType(); }
			static hid_t hdfSpace(const Container& src) {
				hsize_t dims[] = { src.size(), };
				return H5Screate_simple(1, dims, 0);
			}

			static bool checkCompatibility(const Container& src, hid_t dataSet, hid_t hdfMemLayout) {
				const size_t NumDims = 1;
				// check if type of the target dataset and the one stored in this container matches
				htri_t type = H5Tequal(DataType<ElementPOD>::hdfType(), H5Dget_type(dataSet));
				if (type < 1) {
					throw Exception("HDF5 and Container element type declaration does not match");
				}

				// check if rank matches
				int rank = H5Sget_simple_extent_ndims(hdfMemLayout);
				if (rank < 0) {
					throw Exception("Could not get dimensionality of dataset");
				}
				if ( (size_t)rank != NumDims) {
					throw Exception("Rank of source and destination does not match! STL vectors are just of rank 1.");
				}

				// check dimensions
				hsize_t* dims = new hsize_t[rank];
				if (H5Sget_simple_extent_dims(hdfMemLayout, dims, 0) < 0) {
					delete dims;
					throw Exception("Could not retrieve size of dimensions");
				}

				bool sizeFit = dims[0] == src.size();

				// freeing memory
				free(dims);

				if (!sizeFit) {
					throw Exception("Dimensions between dataset and provided container does not match");
				}

				return true;
			}

			static void write(const Container& src, hid_t dataSet, hid_t dataSpace) {
				// checking compatibility of hdf5 target and the c++ src object
				if (!checkCompatibility(src, dataSet, dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::map<..> >::write(): Type compatibility check failed");
				}

				ElementPOD* buffer = (ElementPOD*) malloc(src.size() * DataType<ElementPOD>::size());

				size_t iBuf = 0;
				for (typename Container::const_iterator it = src.begin(); it != src.end(); ++it) {
					DataType<Key>::assignToPOD(it->first, buffer[iBuf].k);
					DataType<Value>::assignToPOD(it->second, buffer[iBuf].v);

					++iBuf;
				}

				H5Dwrite(dataSet, DataType<ElementPOD>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

				// first free POD if necessary (will be handled by each type handler)
//				for (size_t i = 0; i < iBuf; ++i) {
//					DataType<Key>::freePOD(buffer->k);
//					DataType<Value>::freePOD(buffer->v);
//				}
				free(buffer);
			}

			static void read(Container& dst, hid_t dataSet, hid_t dataSpace) {
				const size_t NumDims = 1;
				std::cout << "hdf5::ContainerInterface< std::mapr<..> >::read()" << std::endl;

				if (NumDims != H5Sget_simple_extent_ndims(dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::map<..> >::read(): Dimensions of HDF5 and target container does not match");
				}

				// resize vector to be fitting for data from file
				hsize_t* dims = new hsize_t[NumDims];
				H5Sget_simple_extent_dims(dataSpace, dims, 0);
				size_t nElements = dims[0];
				delete dims;

				// checking compatibility of hdf5 target and the c++ src object
				if (!checkCompatibility(dst, dataSet, dataSpace)) {
					throw Exception("hdf5::ContainerInterface< std::map<..> >::read(): Type compatibility check failed");
				}

				// check if key or value is not a POD
				bool keyComplex = DataType<Key>::isPOD() && !DataType<Key>::isPOD();
				bool valueComplex = DataType<Value>::isPOD() && !DataType<Value>::isPOD();

				ElementPOD* buffer = (ElementPOD*) malloc(dst.size() * DataType<ElementPOD>::size());
				herr_t status = H5Dread(dataSet, DataType<ElementPOD>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
				if (status < 0) {
					free(buffer);
					throw Exception("hdf5::ContainerInterface< std::map<..> >::read(): Error while reading data from file");
				}

				for (size_t i = 0; i < nElements; ++i) {
					Key k;
					Value v;
					if (keyComplex) {
						DataType<Key>::assignFromPOD(buffer[i].k, k);
					}
					else {
						k = buffer[i]->k;
					}

					if (valueComplex) {
						DataType<Value>::assignFromPOD(buffer[i].v, v);
					}
					else {
						v = buffer[i]->v;
					}
					dst[k] = v;
				}

				free(buffer);
			}
	};

	// boost multi_array
	template<typename ElementType, std::size_t NumDims, typename Allocator> struct ContainerInterface< boost::multi_array<ElementType, NumDims, Allocator> > {
		typedef typename boost::multi_array<ElementType, NumDims, Allocator> Container;
		typedef typename std::vector<size_t> Coordinate;

		static hid_t hdfElementType() { return DataType<ElementType>::hdfType(); }

		static hid_t hdfSpace(const Container& src) {
//			std::cout << "  ContainerInterface<boost::multi_array...>::hdfSpace(): Rank=" << NumDims << std::endl;
			hsize_t* dims = new hsize_t[NumDims];
			hsize_t* maxDims = new hsize_t[NumDims];
			for (size_t i = 0; i < NumDims; ++i) {
				dims[i] = src.shape()[i];
				maxDims[i] = H5S_UNLIMITED;

//				std::cout << "  ContainerInterface<boost::multi_array...>::hdfSpace(): dims[" << i << "]=" << dims[i] << ", maxDims[" << i << "]=" << maxDims[i] << std::endl;
			}
			// H5S_UNLIMITED leads to HDF5 C lib errors therefore currently only fixed size is supported
			hid_t spaceId = H5Screate_simple(NumDims, dims, 0);

			free(maxDims);
			free(dims);

//			std::cout << "  ContainerInterface<boost::multi_array...>::hdfSpace(): spaceId=" << spaceId << std::endl;
			return spaceId;
		}

		static Coordinate getArrayExtents(const Container& src, size_t& nElements) {
			Coordinate dimXX(NumDims);
			nElements = 1.;
			for (hsize_t iDim = 0; iDim < NumDims; ++iDim) {
				dimXX[iDim] = static_cast<size_t>(src.shape()[iDim]);
				nElements *= src.shape()[iDim];
			}

			return dimXX;
		}

		static Coordinate getArrayCoordinate(const Container& src, size_t i) {
			Coordinate x(NumDims);
			size_t iBackup = i;
			// calculate coordinate in multi_array
			for (size_t dim = 0; dim < NumDims; ++dim) {
				size_t nom = 1;
				for (size_t ii = dim+1; ii < NumDims; ++ii)
					nom *= src.shape()[ii];

				size_t tmp = iBackup / nom;
				x[dim] = tmp;
				iBackup -= tmp*nom;
			}

			return x;
		}

		static bool checkCompatibility(const Container& src, hid_t dataSet, hid_t hdfMemLayout) {
			// type match
			htri_t type = H5Tequal(DataType<ElementType>::hdfType(), H5Dget_type(dataSet));
			if (type < 1) {
				throw Exception("HDF5 and Container element type declaration does not match");
			}

			// check if rank matches
			int rank = H5Sget_simple_extent_ndims(hdfMemLayout);
			if (rank < 0) {
				throw Exception("Could not get dimensionality of dataset");
			}
			if ( (size_t)rank != NumDims) {
				throw Exception("Rank of source and destination does not match!");
			}

			// check dimensions
			hsize_t* dims = new hsize_t[rank];
			if (H5Sget_simple_extent_dims(hdfMemLayout, dims, 0) < 0) {
				delete dims;
				throw Exception("Could not retrieve size of dimension");
			}

			bool sizeFit = true;
			for (size_t iDim = 0; iDim < NumDims; ++iDim) {
				sizeFit &= (dims[iDim] == src.shape()[iDim]);
			}

			// freeing memory
			free(dims);

			if (!sizeFit) {
				throw Exception("Dimensions between dataset and provided container does not match");
			}

			return true;
		}

		/**
		 * Performs the writing of the src container to the defined dataset
		 * @param src Container to write
		 * @param dataSet HDF5 identifier for the target dataset
		 * @param dataSpace HDF5 identifier fitting the container
		 */
		static void write(const Container& src, hid_t ds, hid_t space) {
			std::cout << "boost::multi_array::write()" << std::endl;

			if (!checkCompatibility(src, ds, space)) {
				throw Exception("ContainerInterface< boost::multi_array<...> >::write(): Type compatibility check failed");
			}

			// check DataType
			if (DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD()) {
				// ok that's no very efficient but we have to copy the elements twice
				// firstly we have to get the POD equivalent of the source type and fill it
				// before writing data out
				typedef typename DataType<ElementType>::PODType POD;
				typedef typename boost::multi_array<POD, NumDims> DstContainer;

				size_t nElements = 1.;
				Coordinate dim = getArrayExtents(src, nElements);

				// this relies on a fitting copy constructor
				// otherwise e.g. gcc would also perform on non-pod but other compilers
				// fail because it is not standard compliant
				DstContainer dst;
				dst.resize(dim);

				// assuming c-data order
				for (size_t i = 0; i < nElements; ++i) {
					Coordinate x = getArrayCoordinate(src, i);
					DataType<ElementType>::assignToPOD(src(x), dst(x));
				}

				H5Dwrite(ds, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, dst.data());

//				for (size_t i = 0; i < nElements; ++i) {
//					Coordinate x = getArrayCoordinate(src, i);
//					DataType<POD>::freePOD(dst(x));
//				}
			}
			else {
				// this allows simple write
				H5Dwrite(ds, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, src.data());
			}
		};

		/**
		 * Reads the data from the dataSet into the dst container
		 * @param src Container to store the data from file
		 * @param dataSet HDF5 identifier for the target dataset
		 * @param dataSpace HDF5 identifier fitting the container
		 */
		static void read(Container& dst, hid_t ds, hid_t space) {
			// get dimensions (no checking required all will be done by checkCompatibility)
			if (NumDims != H5Sget_simple_extent_ndims(space)) {
				throw Exception("ContainerInterface< boost::multi_array<...> >::read(): Dimensions of HDF5 and target container does not match");
			}
			Coordinate dimXX(NumDims);
			size_t nElements = 1.;
			{
				hsize_t* dims = new hsize_t[NumDims];
				H5Sget_simple_extent_dims(space, dims, 0);
				for (hsize_t iDim = 0; iDim < NumDims; ++iDim) {
					dimXX[iDim] = static_cast<size_t>(dims[iDim]);
					nElements *= dims[iDim];
				}
				delete dims;
			}
			// resize multi_array according to source dimensions
			dst.resize(dimXX);

			if (!checkCompatibility(dst, ds, space)) {
				throw Exception("ContainerInterface< boost::multi_array<...> >::read(): Type compatibility check failed");
			}

			// check type of elements stored in container
			// data can only be read to a POD structure therefore with use this...
			typedef typename DataType<ElementType>::PODType POD;

			POD* rawData = (POD*) malloc( DataType<ElementType>::size() * nElements);
			herr_t status = H5Dread(ds, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, rawData);
			if (status < 0) {
				free(rawData);
				throw Exception("hdf5::ContainerInterface< boost::multi_array<..> >::read(): Error while reading data from file");
			}

			for (size_t i = 0; i < nElements; ++i) {
				// assuming c-data order
				Coordinate x = getArrayCoordinate(dst, i);

				if (DataType<ElementType>::isStructType() && !DataType<ElementType>::isPOD()) {
					// .. it target is not a POD we need to translate ..
					DataType<ElementType>::assignFromPOD(rawData[i], dst(x));
				}
				else {
					// .. otherwise we can just copy it from the raw memory
					//TODO: this is very dangerous if ElementType is a pointer!!!
					dst(x) = rawData[i];
				}
			}

			free(rawData);
		}
	};
};

#endif
