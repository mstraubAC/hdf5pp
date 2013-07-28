/*
 * Exception.h
 *
 *  Created on: Jul 17, 2013
 *      Author: marcel
 */

#ifndef HDF5_EXCEPTION_H_
#define HDF5_EXCEPTION_H_

#include <exception>
#include <string>
#include <sstream>

namespace hdf5
{

	class Exception: public std::exception
	{
		public:
			Exception() throw(): msg(""), id(0) {};
			Exception(const std::string& msg, unsigned int id = 0) throw() : msg(msg), id(id) {};
			Exception(const std::stringstream& msg, unsigned int id = 0) throw() : msg(msg.str()), id(id) {};
			virtual ~Exception() throw() {};

			Exception& operator=(const Exception& original) throw() {
				if (this != &original) {
					msg = original.msg;
					id = original.id;
				}
				return *this;
			}

			const char* what() const throw() { return msg.c_str(); }

		private:
			std::string msg;
			unsigned int id;
	};

} /* namespace hdf5 */
#endif /* EXCEPTION_H_ */
