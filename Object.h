/*
 * HdfObject.h
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#ifndef HDF5OBJECT_H_
#define HDF5OBJECT_H_

#include <map>
#include <string>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <hdf5.h>

namespace hdf5
{
	typedef boost::any Attribute;

	class Object
	{
		public:
			typedef boost::shared_ptr<Object> Ptr;
			typedef typename std::map<std::string, Attribute> AttributeMap;
			typedef AttributeMap::const_iterator AttributeConstIterator;
			typedef AttributeMap::iterator AttributeIterator;

			enum ObjectType { All = 0, File = 1, Group = 2, Dataset = 3, Unknown = -1 };

			Object();
			Object(const Object& original);
			virtual ~Object();

			Object& operator=(const Object& original);

			/// returns type of HdfObject
			inline ObjectType getType() const { return fType; }
			inline std::string getTypeName() const;

			// attributes
			inline size_t getNumAttributes() const { return fAttributes.size(); }
			inline bool hasAttribute(const std::string& name) const { return fAttributes.find(name) != fAttributes.end(); }
			inline Attribute& getAttribute(const std::string& name);
			inline const Attribute& getAttribute(const std::string& name) const;
			inline void setAttribute(const std::string& name, const Attribute& value) { fAttributes[name] = value; };
			inline AttributeConstIterator attributesBegin() const { return fAttributes.begin(); }
			inline AttributeConstIterator attributesEnd() const { return fAttributes.end(); }
			inline AttributeIterator attributesBegin() { return fAttributes.begin(); }
			inline AttributeIterator attributesEnd() { return fAttributes.end(); }

		protected:
			ObjectType fType;

		private:
			AttributeMap fAttributes;
	};

} /* namespace hdf5 */
#endif /* HDFOBJECT_H_ */
