/*
 * DataConverter.h
 *
 *  Created on: Jul 25, 2013
 *      Author: marcel
 */

#ifndef HDF5_DATACONVERTER_H_
#define HDF5_DATACONVERTER_H_

#include "Dataset.h"
#include "DataTypes.h"
#include "ContainerInterface.h"
#include <boost/multi_array.hpp>
#include <iostream>
#include <vector>

namespace hdf5 {

	class Vector {
		private:
			double fX, fY, fZ;

		public:
			struct POD {
				double X;
				double Y;
				double Z;
			};
			Vector(): fX(0), fY(0), fZ(0) {};
			Vector(double x, double y, double z): fX(x), fY(y), fZ(z) {}
			Vector(const Vector& v): fX(v.fX), fY(v.fY), fZ(v.fZ) {}
			Vector(const Vector::POD& v): fX(v.X), fY(v.Y), fZ(v.Z) {}
			virtual ~Vector() {};

			inline double getX() const { return fX; }
			inline double getY() const { return fY; }
			inline double getZ() const { return fZ; }

			inline Vector& setX(double in) { fX = in; return *this; }
			inline Vector& setY(double in) { fY = in; return *this; }
			inline Vector& setZ(double in) { fZ = in; return *this; }
	};

	template<> struct DataType<Vector> {
			typedef Vector ElementType;
			typedef Vector::POD PODType;

			static hid_t hdfType() {
				hid_t t = H5Tcreate(H5T_COMPOUND, size());
				H5Tinsert(t, "X", HOFFSET(PODType, X), H5T_NATIVE_DOUBLE);
				H5Tinsert(t, "Y", HOFFSET(PODType, Y), H5T_NATIVE_DOUBLE);
				H5Tinsert(t, "Z", HOFFSET(PODType, Z), H5T_NATIVE_DOUBLE);
				return t;
			}
			static hid_t isStructType() { return true; }
			static hid_t isPOD() { return false; }
			static hsize_t size() { return sizeof(PODType); }
			static void assignToPOD(const ElementType& in, PODType& out) {
				out.X = in.getX();
				out.Y = in.getY();
				out.Z = in.getZ();
			}

			static void assignFromPOD(const PODType& in, ElementType& out) {
				out.setX(in.X).setY(in.Y).setZ(in.Z);
			}
	};

};

#endif /* HDF5_DATACONVERTER_H_ */
