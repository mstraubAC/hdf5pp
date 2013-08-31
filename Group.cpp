/*
 * Group.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#include "Group.h"
#include "Exception.h"
#include "Dataset.h"
#include "File.h"
#include <hdf5.h>
#include <stdexcept>

using namespace std;

namespace hdf5
{

	Group::Group()
	{
		// TODO Auto-generated constructor stub
		fType = Object::ObjectType::Group;
	}

	Group::~Group()
	{
		// since all daughters must be closed before we close our own group
		// we have to clear the map on our own
		fDaughters.clear();
		if (fType == ObjectType::Group) {
			H5Gclose(fObjectId);
		}
	}

	Group::Group(hid_t objectId, const std::string& groupName)
	{
		fName = groupName;
		fType = Object::ObjectType::Group;
		cout << "Group::Group: Calling updateGroup(" << groupName << ", " << objectId << ")" << endl;
		updateGroup(objectId);
		cout << "Group::Group: Calling updateAttributes()" << endl;
		updateAttributes();
	}

	Object::Ptr Group::getObject(const std::string& objectName)
	{
		ObjectIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	Object::ConstPtr Group::getObject(const std::string& objectName) const
	{
		ObjectConstIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	Dataset::Ptr Group::getDataSet(const std::string& dsName) {
		Object::Ptr obj = getObject(dsName);
		Dataset::Ptr g = boost::dynamic_pointer_cast<hdf5::Dataset>(obj);
		if (g.get()) {
			return g;
		}
		else {
			throw Exception("Requested dataset \"" + dsName + "\" does not exist");
		}
	}

	Dataset::ConstPtr Group::getDataSet(const std::string& dsName) const {
		Object::ConstPtr obj = getObject(dsName);
		Dataset::ConstPtr g = boost::dynamic_pointer_cast<const hdf5::Dataset>(obj);
		if (g.get()) {
			return g;
		}
		else {
			throw Exception("Requested dataset \"" + dsName + "\" does not exist");
		}
	}

	Group::Ptr Group::getGroup(const std::string& groupName) {
		Object::Ptr obj = getObject(groupName);
		Group::Ptr g = boost::dynamic_pointer_cast<Group>(obj);
		if (g.get()) {
			return g;
		}
		else {
			throw Exception("Requested group \"" + groupName + "\" does not exist");
		}
	}

	Group::ConstPtr Group::getGroup(const std::string& groupName) const {
		Object::ConstPtr obj = getObject(groupName);
		Group::ConstPtr g = boost::dynamic_pointer_cast<const Group>(obj);
		if (g.get()) {
			return g;
		}
		else {
			throw Exception("Requested group \"" + groupName + "\" does not exist");
		}
	}

	bool Group::deleteObject(const std::string& name)
	{
		if (!hasObject(name)) {
			throw Exception("Group::deleteObject: Object with name '" + name + "' does not exist");
		}

		// this closes the object and removes it from internal list
		fDaughters.erase(name);

		// remove the object from the file
		return H5Gunlink(fObjectId, name.c_str()) > -1;
	}

	void Group::updateGroup(hid_t groupId)
	{
		fDaughters.clear();
		fObjectId = groupId;

		hsize_t nObj;
		herr_t err = H5Gget_num_objs(groupId, &nObj);
		if (err < 0)
			throw Exception("Group::updateGroup(): Could not retrieve number of objects in group");

		for (size_t iObj = 0; iObj < nObj; ++iObj) {
			cout << " -------" << endl;
			char* objName;
			ssize_t nameSize = H5Gget_objname_by_idx(groupId, iObj, objName, 0);
			if (nameSize > -1) {
				nameSize++; // to get termination character, too
				objName = new char[nameSize];
				H5Gget_objname_by_idx(groupId, iObj, objName, nameSize);
				string sObjectName(objName);
				delete objName;

				int objType = H5Gget_objtype_by_idx(groupId, iObj);
				hid_t daughterId = -1;
				Object::Ptr objPtr;
				switch(objType) {
					case H5G_GROUP:
						daughterId = H5Gopen1(groupId, sObjectName.c_str());
						objPtr = Object::Ptr(new Group(daughterId, sObjectName));
						break;
					case H5G_DATASET:
						daughterId = H5Dopen1(groupId, sObjectName.c_str());
						cout << "Group::updateGroup(" << groupId << "): Found H5G_DATASET (named '" << sObjectName << "'), id=" << daughterId << endl;
						objPtr = Object::Ptr(new hdf5::Dataset(daughterId, sObjectName));
						break;
					case H5G_TYPE:
						throw Exception("Group::updateGroup(): Object not implemented H5G_TYPE");
						break;
					case H5G_LINK:
						throw Exception("Group::updateGroup(): Object not implemented H5G_LINK");
						break;
					case H5G_UDLINK:
						throw Exception("Group::updateGroup(): Object not implemented H5G_UDLINK");
						break;
				}

				if (daughterId < 0) {
					cerr << "Could not open daughter '" << sObjectName << "': " << daughterId << endl;
					continue;
				}
				fDaughters[sObjectName] = objPtr;
			}
			else {
				throw Exception("Group::updateGroup(): Could not retrieve objects in group");
			}
		}
	}

} /* namespace hdf5 */
