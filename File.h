/*
 * File.h
 *
 *  Created on: Jul 16, 2013
 *      Author: marcel
 */

#ifndef HDF5FILE_H_
#define HDF5FILE_H_

#include "Group.h"
#include <hdf5.h>
#include <string>

namespace hdf5
{
	/**
	 *
	 */
	struct OpenFile {
			std::string fFileName;
			bool fTruncate;
			bool fCreate;
			bool fRead;
			bool fWrite;

			OpenFile(): fFileName(""), fTruncate(false), fCreate(false), fRead(true), fWrite(false) {};
			OpenFile(const std::string& fileName): fFileName(fileName), fTruncate(false), fCreate(false), fRead(true), fWrite(false) {};
			OpenFile(const OpenFile& original) { operator=(original); }
			OpenFile& operator=(const OpenFile& original);

			/// sets filename
			inline OpenFile& fileName(const std::string& fileName) { fFileName = fileName; return *this; }
			/// set file mode to read & write
			inline OpenFile& readWrite() { fRead = true; fWrite = true; return *this; }
			/// overwrite an existing file
			inline OpenFile& overwrite() { fTruncate = true; return *this; }
			/// create a file if it does not already exist
			inline OpenFile& create() { fCreate = true; return *this; }
			/// does not create the file if it does not already exist
			inline OpenFile& dontCreate() { fCreate = false; return *this; }
	};

	/**
	 * Class encapsulating the HDF5 file API
	 */
	class File : public Group
	{
		public:
			File();
			File(const OpenFile& fileMode): fFile(-1) { openFile(fileMode); }
			File(const File& original) { operator=(original); };

			/**
			 * Constructor opens fileName in read-only mode.
			 * @param fileName
			 */
			File(const std::string& fileName);
			virtual ~File();

			File& operator=(const File& original);

			inline bool isOpen() const { return fFile > -1; }
			inline bool isReadOnly() const;
			/**
			 * Returns the size of the entire file (not only HDF5 portion of file)
			 * @return Size of the file in bytes
			 */
			hsize_t getFileSize() const;
			/**
			 * Returns the amount of space that is unused by any objects in the file.
			 *
			 * Currently, the HDF5 library only tracks free space in a file from a file
			 * open or create until that file is closed, so this routine will only
			 * report the free space that has been created.
			 *
			 * @return Amount of free space in bytes
			 */
			hsize_t getFreeSpace() const;

			/**
			 * closeFile terminates access to an HDF5 file by flushing all data
			 * to storage and terminating access to the file through file_id.
			 *
			 * If this is the last file identifier open for the file and no other
			 * access identifier is open (e.g., a dataset identifier, group identifier,
			 * or shared datatype identifier), the file will be fully closed and access
			 * will end.
			 *
			 * Delayed close:
			 * Note the following deviation from the above-described behavior. If
			 * closeFile is called for a file but one or more objects within the file
			 * remain open, those objects will remain accessible until they are
			 * individually closed. Thus, if the dataset data_sample is open when closeFile
			 * is called for the file containing it, data_sample will remain open and
			 * accessible (including writable) until it is explicitly closed. The file
			 * will be automatically closed once all objects in the file have been closed.
			 *
			 * Be warned, however, that there are circumstances where it is not possible to
			 * delay closing a file. For example, an MPI-IO file close is a collective call;
			 * all of the processes that opened the file must close it collectively. The file
			 * cannot be closed at some time in the future by each process in an independent
			 * fashion. Another example is that an application using an AFS token-based file
			 * access privilege may destroy its AFS token after closeFile has returned
			 * successfully. This would make any future access to the file, or any object
			 * within it, illegal.
			 *
			 * In such situations, applications must close all open objects in a file before
			 * calling closeFile. It is generally recommended to do so in all cases.
			 * @return reference to this object
			 */
			File& closeFile();
			File& openFile(const OpenFile& fileMode);
			/// opens the file in read-only mode
			File& openFile(const std::string& fileName) { return openFile(OpenFile(fileName)); }

		private:
			File(const Object& original);

			OpenFile fFileMode;
			// handles
			hid_t fFile;
	};

} /* namespace hdf5 */
#endif /* FILE_H_ */
