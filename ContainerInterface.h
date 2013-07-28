#ifndef HDF5_CONTAINERINTERFACE_H_
#define HDF5_CONTAINERINTERFACE_H_

#include <hdf5.h>
#include "Exception.h"

namespace hdf5 {
	template<typename T> struct ContainerInterface {
			typedef T Container;

			static hid_t hdfElementType() { return -1; }
			static hid_t hdfSpace(const Container& src) { return -1; }
			static void write(const Container& src, hid_t dataSet, hid_t dataSpace);
			static void read(Container& dst, hid_t dataSet, hid_t dataSpace);
	};

	// stl containers

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
			}
			else {
				// this allows simple write
				H5Dwrite(ds, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, src.data());
			}
		};

		static void read(Container& dst, hid_t ds, hid_t space) {
			herr_t status;

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


			typedef typename DataType<ElementType>::ElementType Element;
			Element*  rawData = (Element*) malloc( DataType<ElementType>::size() * nElements );
			status = H5Dread(ds, DataType<ElementType>::hdfType(), H5S_ALL, H5S_ALL, H5P_DEFAULT, rawData);
			if (status < 0) {
				throw Exception("ContainerInterface< boost::multi_array<...> >::read(): Error while reading data from file");
			}

			// assuming c-data order
			for (size_t i = 0; i < nElements; ++i) {
				Coordinate x = getArrayCoordinate(dst, i);

				// copying data from raw to dst
				// for non POD compound DataTypes this relies on
				// a fitting copy constructor
				dst(x) = rawData[i];
			}

			// freeing memory
			free(rawData);
		}
	};
};

#endif
