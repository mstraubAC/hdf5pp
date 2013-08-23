#include<string>
#include<iostream>
#include<sstream>
#include<vector>
#include<limits>
#include<boost/multi_array.hpp>
#include <boost/any.hpp>

#include "Object.h"
#include "File.h"

using namespace std;

//template<typename T> class Vector {
//	public:
//	T x;
//	T y;
//	T z;
//
//	Vector& operator=(const Vector& in) {
//		if (this != &in) {
//			x = in.x;
//			y = in.y;
//			z = in.z;
//		}
//		return *this;
//	}
//
//	static H5::CompType type() {
//		H5::CompType vecType( sizeof(Vector<T>) );
//		vecType.insertMember("X", HOFFSET(Vector<T>, x), H5::PredType::NATIVE_DOUBLE);
//		vecType.insertMember("Y", HOFFSET(Vector<T>, y), H5::PredType::NATIVE_DOUBLE);
//		vecType.insertMember("Z", HOFFSET(Vector<T>, z), H5::PredType::NATIVE_DOUBLE);
//		return vecType;
//	}
//
//	void write(H5::H5File& file, const std::string& name) {
//
//	};
//};



int main (int argc, char** argv) {
	cout << "HDF5 Test program for c++ and compound datatypes" << endl;

	hdf5::File file = hdf5::OpenFile("test.h5").readWrite();
	cout << " ========= File has been opened ========= " << endl;
	cout << "File size    = " << file.getFileSize() << endl;
	cout << "Free space   = " << file.getFreeSpace() << endl;
	cout << "# Attributes = " << file.getNumAttributes() << endl;
	cout << "# Objects    = " << file.getNumObjects() << endl;
	cout << " ---" << endl;
	const hdf5::Object& testTable = dynamic_cast<hdf5::Group&>(file("TestGroup"))("testData 2d");
	cout << "DataSet: " << testTable.getName() << endl;
	cout << "     --> #Attributes=" << testTable.getNumAttributes() << endl;
	for (hdf5::Object::AttributeConstIterator it = testTable.attributesBegin(); it != testTable.attributesEnd(); ++it) {
		cout << "         * " << it->first << ": " << it->second << endl;

	}
	const hdf5::Dataset& dsTest = dynamic_cast<const hdf5::Dataset&>(testTable);
	boost::multi_array<int32_t, 2> array;
	dsTest.read(array);
	for (size_t x = 0; x < array.shape()[0]; ++x) {
		for (size_t y = 0; y < array.shape()[0]; ++y) {
			cout << array[x][y] << ", ";
		}
		cout << endl;
	}
//	group->getObject("testData 2d");

	cout << "Creating boost::multi_array<double, 2>" << endl;
	boost::multi_array<double, 2> arrayOut(boost::extents[10][10]);
	for (size_t x = 0; x < arrayOut.shape()[0]; ++x) {
		for (size_t y = 0; y < arrayOut.shape()[0]; ++y) {
			arrayOut[x][y] = x*y;
		}
	}

	try {
		cout << "Delete /TestGroup/multArray_WriteTest if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("multArray_WriteTest");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout dataset" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("multArray_WriteTest", arrayOut);
	cout << "Writeout dataset --> DONE" << endl;

	//
	cout << " ########### writing compound data type" << endl;
	boost::multi_array<hdf5::Vector, 2> arrayOutCompound(boost::extents[10][10]);
	for (size_t x = 0; x < arrayOutCompound.shape()[0]; ++x) {
		for (size_t y = 0; y < arrayOutCompound.shape()[0]; ++y) {
			arrayOutCompound[x][y] = hdf5::Vector(x, y, x*y);
		}
	}
	try {
		cout << "Delete /TestGroup/multArray_WriteTestCompound if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("multArray_WriteTestCompound");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout compound dataset" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("multArray_WriteTestCompound", arrayOutCompound);
	cout << "Writeout dataset --> DONE" << endl;

	// std::vector
	cout << " ########### writing vector" << endl;
	vector<double> testVec;
	for (size_t x = 0; x < 10; ++x) {
		testVec.push_back(x);
	}
	try {
		cout << "Delete /TestGroup/testVec if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("testVec");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("testVec", testVec);
	cout << "Writeout dataset --> DONE" << endl;

	// std::list
	cout << " ########### writing list" << endl;
	list<double> testList;
	for (size_t x = 0; x < 10; ++x) {
		testList.push_back(x);
	}
	try {
		cout << "Delete /TestGroup/testList if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("testList");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("testList", testList);
	cout << "Writeout dataset --> DONE" << endl;

	// std::map
	// std::map
	cout << " ########### writing map<double, int>" << endl;
	map<double, int> testMap;
	for (size_t x = 0; x < 10; ++x) {
		testMap[double(x)] = x;
	}
	try {
		cout << "Delete /TestGroup/testList if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("testMap");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("testMap", testMap);
	cout << "Writeout dataset --> DONE" << endl;

	cout << " ########### writing map<string, int>" << endl;
	map<string, double> testMapStr;
	for (size_t x = 0; x < 16; ++x) {
		stringstream a;
		a << "Diese Zahl ist in hex " << hex << x << dec;
		testMapStr[a.str()] = x;
	}
	testMapStr["Katzenklo, Katzenkla. Ja das macht die Katze froh. Wir gruessen Helge S."] = 2222;
	testMapStr["Die Antwort auf alles"] = 42;
	testMapStr["zz top and la Grange"] = 23;
	try {
		cout << "Delete /TestGroup/testList if it exists" << endl;
		dynamic_cast<hdf5::Group&>(file("TestGroup")).deleteObject("testMapStr");
	}
	catch (const hdf5::Exception& e) {
	}

	cout << "Writeout" << endl;
	dynamic_cast<hdf5::Group&>(file("TestGroup")).createDataset("testMapStr", testMapStr);
	cout << "Writeout dataset --> DONE" << endl;

	cout << " --- Clearing map<string, int>" << endl;
	testMapStr.clear();
	dynamic_cast<hdf5::Dataset&>(dynamic_cast<hdf5::Group&>(file("TestGroup"))("testMapStr")).read(testMapStr);
	for (map<string, double>::const_iterator it = testMapStr.begin(); it != testMapStr.end(); ++it) {
		cout << " * " << it->first << " --> " << it->second << endl;
	}

	cout << " ======= THE END ======= " << endl;

//	// opening file
//	H5::H5File outFile("test.h5", H5F_ACC_TRUNC);
//
//	typedef Vector<double> VecDouble;
//	VecDouble v;
//	v.x = 1;
//	v.y = 2;
//	v.z = 3;
//
//	// store single value
//	hsize_t dimensionsSingle[] = { 1, };
//	H5::DataSpace dataSpaceSingle(1, dimensionsSingle);
//	H5::DataSet dataSetSingle = outFile.createDataSet("single", v.type(), dataSpaceSingle);
//	dataSetSingle.write(&v, v.type());
//
//	// now we store an stl vector of vectors
//	vector<VecDouble> vv;
//	for (size_t i = 0; i < 20; ++i) {
//		VecDouble x;
//		x.x = static_cast<double>(i)+.1;
//		x.y = static_cast<double>(i)+.2;
//		x.z = static_cast<double>(i)+.3;
//
//		vv.push_back(x);
//	}
//
////	for (vector<VecDouble>::const_iterator s = vv.begin(); s != vv.end(); ++s) {
////		cout << s->x << ", " << s->y << ", " << s->z << endl;
////	}
//	hsize_t dimensionsVec[] = { vv.size(), };
//	H5::DataSpace dataSpaceVec(1, dimensionsVec);
//	H5::DataSet dataSetVec = outFile.createDataSet("vec", v.type(), dataSpaceVec);
//	dataSetVec.write(vv.data(), v.type());
//
//	// boost multi_array
//	boost::multi_array<VecDouble, 3> field;
//	field.resize(boost::extents[4][4][4]);
//	for (size_t ix = 0; ix < field.shape()[0]; ++ix) {
//		for (size_t iy = 0; iy < field.shape()[1]; ++iy) {
//			for (size_t iz = 0; iz < field.shape()[2]; ++iz) {
//				VecDouble x;
//				x.x = static_cast<double>(ix);
//				x.y = static_cast<double>(iy);
//				x.z = static_cast<double>(iz);
//				field[ix][iy][iz] = x;
//			}
//		}
//	}
//	hsize_t dimensionsField[] = { field.shape()[0], field.shape()[1], field.shape()[2] };
//	H5::DataSpace dataSpaceField(3, dimensionsField);
//	H5::DataSet dataSetField = outFile.createDataSet("field", v.type(), dataSpaceField);
//	dataSetField.write(field.data(), v.type());
//
//	// closing file
//	outFile.close();
//
//	/*
//	 * Now reopen file for reading
//	 */
//	cout << endl << " --- Now read elements from file" << endl;
//	cout << " - Processing field:"  << endl;
//	H5::H5File infile("test.h5", H5F_ACC_RDONLY);
//	H5::DataSet dsetRoField(infile.openDataSet("field"));
//	H5::DataSpace dspaceRoField(dsetRoField.getSpace());
//
//	// getting rank and dimension of field
//	int rankField = dspaceRoField.getSimpleExtentDims(0);
//	if (rankField < 1)
//		abort();
//	cout << "   * rank=" << rankField << endl;
//	hsize_t dimField[rankField];
//	dspaceRoField.getSimpleExtentDims(dimField);
//	cout << "   * dims=";
//	for (size_t i = 0; i < static_cast<size_t>(rankField); ++i) {
//		cout << dimField[i] << "x";
//	}
//	cout << endl;
//
//	// preparing field for data
//	if (rankField > 3) {
//		cerr << "Unsupported rank " << rankField << " should be 2" << endl;
//		return 1;
//	}
//	boost::multi_array<VecDouble, 3> fieldRo;
//	fieldRo.resize( boost::extents[ dimField[0] ][ dimField[1] ][ dimField[2] ] );
//	dsetRoField.read(fieldRo.data(), v.type());
//	infile.close();
//	for (size_t iz = 0; iz < dimField[2]; ++iz) {
//		cout << "* z=" << iz << endl;
//		for (size_t ix = 0; ix < dimField[0]; ++ix) {
//			for (size_t iy = 0; iy < dimField[1]; ++iy) {
//				cout << "(" << fieldRo[ix][iy][iz].x << ", " << fieldRo[ix][iy][iz].y << ", " << fieldRo[ix][iy][iz].z << ") ";
//			}
//			cout << endl;
//		}
//	}
//
//
//	return 0;
//	for (unsigned int xx = 0; xx < numeric_limits<unsigned int>::max(); ++xx) {
//
//		double* x = new double[66666];
//		for (size_t i = 0; i < 66666; ++i) {
//			double a = sin(static_cast<double>(i) * 3.14156 / 66666);
//			double b = sin(static_cast<double>(i) * 3.14156 / 66666);
//			double c = a*a + b*b;
//			double d = sqrt(c);
//
//			x[i] = d;
//			fieldRo.resize( boost::extents[ 5 ][ 5 ][ 5 ] );
//		}
//		delete x;
//
//
//	}
}
