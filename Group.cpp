/*
 * Group.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#include "Group.h"
#include "Exception.h"
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
		// TODO Auto-generated destructor stub
	}


	inline Object::Ptr Group::operator ()(const std::string& objectName)
	{
		ObjectIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	inline const Object::Ptr Group::operator ()(const std::string& objectName) const
	{
		ObjectConstIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	Group::Group(hid_t objectId, const std::string& groupName)
	{
		fName = groupName;
		fType = Object::ObjectType::Group;
		updateGroup(objectId);
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
			char* groupName;
			ssize_t nameSize = H5Gget_objname_by_idx(groupId, iObj, groupName, 0);
			if (nameSize > -1) {
				nameSize++; // to get termination character, too
				groupName = new char[nameSize];
				H5Gget_objname_by_idx(groupId, iObj, groupName, nameSize);
				string sGroupName(groupName);
				delete groupName;

//				string objectPath(fName + "/" + sGroupName);
//				string objectPath(sGroupName);

				int objType = H5Gget_objtype_by_idx(groupId, iObj);
				switch(objType) {
					case H5G_GROUP:
					{
//						cout << "Group::updateGroup(" << groupId << "): Found H5G_GROUP (named '" << sGroupName << "')";
						hid_t daughterGroupId = H5Gopen1(groupId, sGroupName.c_str());
						if (daughterGroupId < 0) {
							cerr << "Could not open daughter Group: " << daughterGroupId << endl;
							continue;
						}
//						cout << " with id=" << daughterGroupId << endl;
						Object::Ptr g(new Group(daughterGroupId, sGroupName));
						fDaughters[sGroupName] = g;
					}
						break;
					case H5G_DATASET:
					{
						throw Exception("Group::updateGroup(): Object not implemented H5G_DATASET");

					}
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
			}
			else {
				throw Exception("Group::updateGroup(): Could not retrieve objects in group");
			}
		}
	}

} /* namespace hdf5 */
