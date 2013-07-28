/*
 * Group.h
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#ifndef HDF5_GROUP_H_
#define HDF5_GROUP_H_

#include "Object.h"
#include "Dataset.h"
#include "DataConverter.h"
#include <boost/concept_check.hpp>

namespace hdf5
{

	class Group: public hdf5::Object
	{
		public:
			typedef boost::shared_ptr<Group> Ptr;
			typedef boost::shared_ptr<const Group> ConstPtr;
			typedef typename std::map<std::string, Object::Ptr> ObjectMap;
			typedef ObjectMap::const_iterator ObjectConstIterator;
			typedef ObjectMap::iterator ObjectIterator;

			Group();
			virtual ~Group();


			Object::Ptr getObject(const std::string& objectName);
			const Object::Ptr getObject(const std::string& objectName) const;

			Object& operator()(const std::string& objectName) { return *(getObject(objectName).get()); };
			const Object& operator()(const std::string& objectName) const { return *(getObject(objectName).get()); };

			/// returns the number hdf5 objects stored in this one
			inline size_t getNumObjects() const { return fDaughters.size(); }
			inline bool hasObject(const std::string& name) const { return fDaughters.find(name) != fDaughters.end(); }

			// iterators over objects in this object
			inline ObjectConstIterator objectsBegin() const { return fDaughters.begin(); }
			inline ObjectConstIterator objectsEnd() const { return fDaughters.end(); }

			inline ObjectIterator objectsBegin() { return fDaughters.begin(); }
			inline ObjectIterator objectsEnd() { return fDaughters.end(); }


			/**
			 * Unlinks the an object from our namespace
			 *
			 * According to the current HDF5 documentation space
			 * is freed if all references to this object are closed.
			 * The freed memory is only reusable before closing the file.
			 *
			 * @param name of object to remove
			 * @return True if unlinking was successfull
			 */
			bool deleteObject(const std::string& name);

			// subobject creation interface
			template<typename T> Dataset::Ptr createDataset(const std::string& name, T& src) {
				// throw an exception, if it already exists
				if (hasObject(name)) {
					throw Exception("Could not create dataset '" + name + "' because it already exists");
				}

				hid_t memType =  ContainerInterface<T>::hdfElementType();
				hid_t fileType = memType;
				hid_t space = ContainerInterface<T>::hdfSpace(src);

				hid_t dsId = H5Dcreate2(fObjectId, name.c_str(), fileType, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
				if (dsId < 0) {
					throw Exception("Could not create dataset '" + name + "'");
				}
				Dataset::Ptr dsPtr = Dataset::Ptr(new hdf5::Dataset(dsId, name));
				dsPtr->write(src);
				fDaughters[name] = dsPtr;
				return dsPtr;
			}
			Group::Ptr createGroup(std::string& name);

		protected:
			Group(hid_t objectId, const std::string& groupName);
			void updateGroup(hid_t groupId);

		private:
			ObjectMap fDaughters;
	};

} /* namespace hdf5 */
#endif /* GROUP_H_ */
