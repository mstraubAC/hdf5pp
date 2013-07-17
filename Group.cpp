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

	}

	Group::~Group()
	{
		// TODO Auto-generated destructor stub
	}


	inline Object& Group::operator ()(const std::string& objectName)
	{
		ObjectIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	inline const Object& Group::operator ()(const std::string& objectName) const
	{
		ObjectConstIterator it = fDaughters.find(objectName);
		if (it != fDaughters.end())
			return it->second;
		else
			throw std::out_of_range("Object \"" + objectName + "\" not found!");
	}

	Group::Group(hid_t groupId, const std::string& groupName)
	{
		fName = groupName;
		updateGroup(groupId);
	}

	void Group::updateGroup(hid_t groupId)
	{
		fDaughters.clear();
		fGroupId = groupId;

		hsize_t nObj;
		herr_t err = H5Gget_num_objs(groupId, &nObj);

		for (size_t iObj = 0; iObj < nObj; ++iObj) {
			char* groupName;
			ssize_t nameSize = H5Gget_objname_by_idx(groupId, iObj, groupName, 0);
			if (nameSize > -1) {
				nameSize++; // to get termination character, too
				groupName = new char[nameSize];
				H5Gget_objname_by_idx(groupId, iObj, groupName, nameSize);
				string sGroupName(groupName);
				delete groupName;

				int objType = H5Gget_objtype_by_idx(groupId, iObj);
				switch(objType) {
					case H5G_GROUP:
					{
						hid_t daughterGroupId = H5Gopen1(groupId, (fName + "/" + sGroupName).c_str());
						Group g(daughterGroupId, sGroupName);
						fDaughters[sGroupName] = g;
					}
						break;
					case H5G_DATASET:
					{
						throw Exception("Group::updateGroup(): Object not implemente H5G_DATASET");

					}
						break;
					case H5G_TYPE:
						throw Exception("Group::updateGroup(): Object not implemente H5G_TYPE");
						break;
					case H5G_LINK:
						throw Exception("Group::updateGroup(): Object not implemente H5G_LINK");
						break;
					case H5G_UDLINK:
						throw Exception("Group::updateGroup(): Object not implemente H5G_UDLINK");
						break;
				}
			}
			else {
				throw Exception("Group::updateGroup(): Could not retrieve objects in group");
			}
		}
	}

} /* namespace hdf5 */
