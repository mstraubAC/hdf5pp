/*
 * File.cpp
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#include "File.h"
#include "Exception.h"
#include <hdf5.h>
#include <sstream>
#include <iostream>

using namespace std;

namespace hdf5
{
	File::File(): fFile(-1)
	{
		// TODO Auto-generated constructor stub

	}

	File::~File()
	{
		closeFile();
	}

	OpenFile& OpenFile::operator =(const OpenFile& original)
	{
		if (this != &original) {
			fFileName = original.fFileName;
			fTruncate = original.fTruncate;
			fCreate = original.fCreate;
			fRead = original.fRead;
			fWrite = original.fWrite;
		}
		return *this;
	}

	File& File::closeFile()
	{
		if (isOpen()) {
			herr_t id = H5Fclose(fFile);
			if (id > -1) {
				fFile = -1;
			}
			else {
				stringstream ss;
				ss << "Could not close file \"" << fFileMode.fFileName << "\": err_id=" << id;
				throw Exception(ss.str());
			}
		}
		return *this;
	}

	File::File(const std::string& fileName): fFile(-1)
	{
		openFile(OpenFile(fileName));
	}

	hsize_t File::getFileSize() const
	{
		if (this->fFile > -1) {
			hsize_t size;
			herr_t err = H5Fget_filesize(fFile, &size);
			if (err > -1) {
				return size;
			}
			else {
				stringstream ss;
				ss << "Error while getting filesize. Id=" << err;
				throw Exception(ss);
			}
		}
		else {
			throw Exception("File is not opened");
		}
	}

	hsize_t File::getFreeSpace() const
	{
		if (this->fFile > -1) {
			hssize_t ret = H5Fget_freespace(fFile);
			if (ret > -1) {
				return static_cast<hsize_t>(ret);
			}
			else {
				stringstream ss;
				ss << "Error while getting filesize. Id=" << ret;
				throw Exception(ss);
			}
		}
		else {
			throw Exception("File is not opened");
		}
	}

	inline bool File::isReadOnly() const
	{
		if (this->fFile > -1) {
			unsigned int intent;
			herr_t ret = H5Fget_intent(fFile, &intent);
			if (ret > -1) {
				return (intent == H5F_ACC_RDONLY);
			}
			else {
				stringstream ss;
				ss << "Error while getting filemode. Id=" << ret;
				throw Exception(ss);
			}
		}
		else {
			throw Exception("File is not opened");
		}
	}

	File& File::openFile(const OpenFile& fileMode)
	{
		closeFile();
		fFileMode = fileMode;

		if (fFileMode.fCreate && fFileMode.fWrite && fFileMode.fRead) {
			unsigned int flags = fFileMode.fTruncate ? H5F_ACC_TRUNC : H5F_ACC_EXCL;
			fFile = H5Fcreate(fFileMode.fFileName.c_str(), flags, H5P_DEFAULT, H5P_DEFAULT);
		}

		if ( (fFile < 0 || !fFileMode.fCreate) ) {
			unsigned int flags = (fFileMode.fRead && !fFileMode.fWrite) ? H5F_ACC_RDONLY : H5F_ACC_RDWR;
			fFile = H5Fopen(fFileMode.fFileName.c_str(), flags, H5P_DEFAULT);
		}

		if (fFile < 0) {
			throw Exception("Could not open file \"" + fFileMode.fFileName + "\"");
		}

		// retrieve object list
		updateGroup(fFile);

		cout << " --- Groups in root:" << endl;
		for (Group::ObjectConstIterator itObj = objectsBegin(); itObj != objectsEnd(); ++itObj) {
			cout << " * name=" << itObj->first << endl;
			if (itObj->second->getType() == Object::Group) {
				Object::Ptr o = itObj->second;
				try {
					Group::Ptr g = boost::dynamic_pointer_cast<Group>(o);
					cout << "    --> n-Objects: " << g->getNumObjects() << endl;
				}
				catch (const std::bad_cast& e) {
					cout << "Could not cast to Group: " << e.what() << endl;
				}
			}
		}


//		ssize_t objCount = H5Fget_obj_count(fFile, H5F_OBJ_ALL);
//		cout << "H5Fget_obj_count=" << objCount << endl;
//
//		hid_t* objIdList = new hid_t[objCount+1];
//		ssize_t objIdCount = H5Fget_obj_ids(fFile, H5F_OBJ_ALL, objCount + 1, objIdList );
//		cout << "H5Fget_obj_ids=" << endl;
//		for (size_t idx = 0; idx < objCount; ++idx) {
//			cout << "id[" << idx << "]: " << objIdList[idx] << endl;
//		}
		return *this;
	}

} /* namespace hdf5 */

