/*
 * Group.h
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#ifndef HDF5GROUP_H_
#define HDF5GROUP_H_

#include "Object.h"

namespace hdf5
{

	class Group: public hdf5::Object
	{
		public:
			typedef boost::shared_ptr<Group> Ptr;
			typedef typename std::map<std::string, Object::Ptr> ObjectMap;
			typedef ObjectMap::const_iterator ObjectConstIterator;
			typedef ObjectMap::iterator ObjectIterator;

			Group();
			virtual ~Group();


			inline Object::Ptr operator()(const std::string& objectName);
			inline const Object::Ptr operator()(const std::string& objectName) const;

			/// returns the number hdf5 objects stored in this one
			inline size_t getNumObjects() const { return fDaughters.size(); }
			inline bool hasObject(const std::string& name) const { return fDaughters.find(name) != fDaughters.end(); }

			// iterators over objects in this object
			inline ObjectConstIterator objectsBegin() const { return fDaughters.begin(); }
			inline ObjectConstIterator objectsEnd() const { return fDaughters.end(); }

			inline ObjectIterator objectsBegin() { return fDaughters.begin(); }
			inline ObjectIterator objectsEnd() { return fDaughters.end(); }

		protected:
			Group(hid_t groupId, const std::string& groupName);
			void updateGroup(hid_t groupId);

		private:
			hid_t fGroupId;
			ObjectMap fDaughters;
			std::string fName;
	};

} /* namespace hdf5 */
#endif /* GROUP_H_ */
