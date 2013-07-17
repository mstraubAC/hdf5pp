/*
 * HdfObject.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#include "Object.h"
#include <exception>
#include <stdexcept>


using namespace std;

namespace hdf5
{

	Object::Object() : fType(Unknown)
	{
		// TODO Auto-generated constructor stub

	}

	Object::Object(const Object& original)
	{
		operator=(original);
	}

	Object::~Object()
	{
		// TODO Auto-generated destructor stub
	}

	Object& Object::operator=(const Object& original)
	{
		if (this != &original) {
			fType = original.fType;
		}
		return *this;
	}

	inline Attribute& Object::getAttribute(const std::string& name)
	{
		AttributeIterator it = fAttributes.find(name);
		if (it != fAttributes.end())
			return it->second;
		else
			throw std::out_of_range("Attribute \"" + name + "\" not found!");
	}

	inline const Attribute& Object::getAttribute(const std::string& name) const
	{
		AttributeConstIterator it = fAttributes.find(name);
		if (it != fAttributes.end())
			return it->second;
		else
			throw std::out_of_range("Attribute \"" + name + "\" not found!");
	}
} /* namespace hdf5 */
