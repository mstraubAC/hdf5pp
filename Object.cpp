/*
 * HdfObject.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#include "Object.h"
#include "Exception.h"
#include <exception>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <typeinfo>


using namespace std;

namespace hdf5
{

	Object::Object() : fType(Unknown), fObjectId(-1)
	{
		// TODO Auto-generated constructor stub

	}

	Object::Object(const Object& original)
	{
		operator=(original);
	}

	Object::~Object() {}

	Object& Object::operator=(const Object& original)
	{
		if (this != &original) {
			fType = original.fType;
		}
		return *this;
	}

	void Object::updateAttributes()
	{
		fAttributes.clear();

		if (fObjectId < 0)
			return;

		// get attributes
		int nAttrs = H5Aget_num_attrs(fObjectId);
		if (nAttrs < 0) {
			throw Exception("Could not retrieve number of attributes");
		}

		for (size_t iAttr = 0; iAttr < static_cast<size_t>(nAttrs); ++iAttr) {
			char* attrName;
			char objName[] = ".";
			ssize_t nameLen = H5Aget_name_by_idx(fObjectId, objName, H5_INDEX_NAME, H5_ITER_INC, iAttr, 0, 0, H5P_DEFAULT);

			if (nameLen > 0) {
				// fetch name of this attribute
				attrName = new char[++nameLen];
				H5Aget_name_by_idx(fObjectId, objName, H5_INDEX_NAME, H5_ITER_INC, iAttr, attrName, nameLen, H5P_DEFAULT);
				string sAttrName(attrName);
				delete[] attrName;

				// open attribute
				hid_t attrId = H5Aopen_by_idx(fObjectId, objName, H5_INDEX_NAME, H5_ITER_INC, iAttr, H5P_DEFAULT, H5P_DEFAULT);
				if (attrId < 0) {
					cerr << "Could not open attribute '" << sAttrName << "'" << endl;
					continue;
				}

				// parse type info
//				cout << "Reading attribute: " << sAttrName << endl;
				fAttributes[sAttrName] = llReadAttribute(attrId);

				// close attribute
				H5Aclose(attrId);
			}
		}
	}


	Object::TypeName Object::getStlType(hid_t hType) const
	{
		H5T_class_t attrType = H5Tget_class(hType);

		// parsing type info
		stringstream typeName;
		switch(attrType) {
			case H5T_INTEGER:
			{
				size_t precision = H5Tget_precision(hType);
				H5T_sign_t sign = H5Tget_sign(hType);
				switch (sign) {
					case H5T_SGN_NONE:
						typeName << "u";
						break;
					case H5T_SGN_2:
						break;
					case H5T_NSGN:
						break;
					case H5T_SGN_ERROR:
						break;
				}
				typeName << "int" << dec << precision << "_t";
			}
				break;
			case H5T_FLOAT:
			{
				size_t precision = H5Tget_precision(hType);
				if (precision == 32) {
					typeName << "float";
				}
				else if (precision == 64) {
					typeName << "double";
				}
				else {
					stringstream ss;
					ss << "Object::getStlType: H5T_FLOAT with unkown precision " << precision << "bits";
					throw Exception(ss);
				}
			}
				break;
			case H5T_STRING:
			{
				H5T_cset_t charSet = H5Tget_cset(hType);
				if (charSet != 0) {
					stringstream ss;
					ss << "Object::getStlType: Unknown character encoding type '" << charSet << "'. Currently only US-ASCII is supported.";
					throw Exception(ss);
				}
				typeName << "string";
			}
				break;
			case H5T_BITFIELD:
				throw Exception("Object::getStlType: Unknown type H5T_BITFIELD");
				break;
			case H5T_OPAQUE:
				throw Exception("Object::getStlType: Unknown type H5T_OPAQUE");
				break;
			case H5T_COMPOUND:
				throw Exception("Object::getStlType: Unknown type H5T_COMPOUND");
				break;
			case H5T_REFERENCE:
				throw Exception("Object::getStlType: Unknown type H5T_REFERENCE");
				break;
			case H5T_ENUM:
				throw Exception("Object::getStlType: Unknown type H5T_ENUM");
				break;
			case H5T_VLEN:
				throw Exception("Object::getStlType: Unknown type H5T_VLEN");
				break;
			case H5T_ARRAY:
				throw Exception("Object::getStlType: Unknown type H5T_ARRAY");
				break;
			case H5T_NO_CLASS:
				throw Exception("Object::getStlType: Unknown type H5T_NO_CLASS");
				break;
			case H5T_TIME:
				throw Exception("Object::getStlType: Unknown type H5T_TIME");
				break;
			case H5T_NCLASSES:
				throw Exception("Object::getStlType: Unknown type H5T_NCLASSES");
				break;
		}

		return typeName.str();
	}

} /* namespace hdf5 */
