/*
 * Dataset.h
 *
 *  Created on: Jul 18, 2013
 *      Author: marcel
 */

#ifndef HDF5_DATASET_H_
#define HDF5_DATASET_H_

#include "Object.h"
#include "DataConverter.h"
#include <string>

namespace hdf5
{
	class Dataset: public hdf5::Object
	{
		public:
			typedef typename boost::shared_ptr<Dataset> Ptr;
			typedef typename boost::shared_ptr<const Dataset> ConstPtr;

			Dataset();
			virtual ~Dataset();

			size_t getRank() const;
			size_t getDimension(size_t dim) const;

			/// returns the HDF5 identifier to directly use C methods on the dataset
			hid_t getIdentifier() const { return fObjectId; }

			template<typename T> bool read(T& dst) const {
				ContainerInterface<T>::read(dst, fObjectId, fSpace);
				return true;
			}
			template<typename T> bool write(const T& src) {
				ContainerInterface<T>::write(src, fObjectId, fSpace);
				return true;
			}


		protected:
			friend class Group;
			Dataset(hid_t objectId, const std::string& name);

		private:
			hid_t fSpace;
	};

} /* namespace hdf5 */
#endif /* DATASET_H_ */
