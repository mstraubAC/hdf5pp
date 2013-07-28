/*
 * HdfObject.h
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#ifndef HDF5_OBJECT_H_
#define HDF5_OBJECT_H_

#include "hdfLLReading.h"
#include <map>
#include <string>
#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>
#include <hdf5.h>

namespace hdf5
{
	typedef any Attribute;

//	struct Type {
//			std::type_info typeInfo;
//			std::string typeName;
//	};

	class Object
	{
		public:
			typedef std::string TypeName;
			typedef boost::shared_ptr<Object> Ptr;
			typedef typename std::map<std::string, Attribute> AttributeMap;
			typedef AttributeMap::const_iterator AttributeConstIterator;
			typedef AttributeMap::iterator AttributeIterator;

			enum ObjectType { All = 0, File = 1, Group = 2, Dataset = 3, Unknown = -1 };

			Object();
			Object(const Object& original);
			virtual ~Object();

			Object& operator=(const Object& original);

			const std::string& getName() const { return fName; }

			/// returns type of HdfObject
			inline ObjectType getType() const { return fType; }
			inline std::string getTypeName() const;

			// attributes
			inline size_t getNumAttributes() const { return fAttributes.size(); }
			inline bool hasAttribute(const std::string& name) const { return fAttributes.find(name) != fAttributes.end(); }
			template <typename T> T& getAttribute(const std::string& name) {
				AttributeIterator it = fAttributes.find(name);
				if (it != fAttributes.end())
					return boost::any_cast<T&>(it->second);
				else
					throw std::out_of_range("Attribute \"" + name + "\" not found!");
			}
			template <typename T> const T& getAttribute(const std::string& name) const {
				AttributeConstIterator it = fAttributes.find(name);
				if (it != fAttributes.end())
					return boost::any_cast<const T&>(it->second);
				else
					throw std::out_of_range("Attribute \"" + name + "\" not found!");
			}
			inline void setAttribute(const std::string& name, const Attribute& value) { fAttributes[name] = value; };
			inline AttributeConstIterator attributesBegin() const { return fAttributes.begin(); }
			inline AttributeConstIterator attributesEnd() const { return fAttributes.end(); }
			inline AttributeIterator attributesBegin() { return fAttributes.begin(); }
			inline AttributeIterator attributesEnd() { return fAttributes.end(); }

		protected:
			ObjectType fType;
			std::string fName;
			hid_t fObjectId;

			void updateAttributes();

			TypeName getStlType(hid_t hdfTypeId) const;

		private:
			AttributeMap fAttributes;
	};

} /* namespace hdf5 */
#endif /* HDFOBJECT_H_ */
