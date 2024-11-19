/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _ndarray_H
#define _ndarray_H

#include "stacktrace.h"
#include "vector_utils.h"

//It is best to define CNDARRAY_BOUNDSCHECK as a compiler preprocessor definition so that it is defined across all source units
//#define CNDARRAY_BOUNDSCHECK
//#undef CNDARRAY_BOUNDSCHECK

template<typename T, size_t ND>
class cNDArray{

private:
	std::vector<T> datastore;		
	std::vector<cNDArray<T, ND - 1>> hdims;
	T* pdata = (T*) NULL;

	void allocate(const std::vector<size_t>& dims){
		_GSTITEM_
		size_t n = 1;
		for (size_t i = 0; i < dims.size(); i++) n*=dims[i];
		datastore.resize(n);
		pdata = datastore.data();		
		return;
	}

	bool create_higher_dims(const std::vector<size_t>& dims, const size_t& thisdim, T* dataptr){
		_GSTITEM_
		pdata = dataptr;

		size_t n = 1;
		for (size_t i = thisdim; i < dims.size(); i++) n *= dims[i];
		size_t stride = n / dims[thisdim];

		cNDArray<T, ND - 1> c(dims, thisdim, pdata);
		hdims.push_back(c);

		T* p = dataptr;
		hdims.resize(dims[thisdim]);
		for (size_t i = 0; i < hdims.size(); i++){
			hdims[i].initialise(dims, thisdim + 1, p);
			p += stride;
		}		
		return true;
	}

public:

	cNDArray(){
		
	}

	cNDArray(const std::vector<size_t>& dims, const size_t thisdim = 0, T* dataptr=NULL){
		_GSTITEM_
		initialise(dims, thisdim, dataptr);		
	}

	cNDArray(const cNDArray& rhs){
		_GSTITEM_
		datastore = rhs.datastore;
		std::vector<size_t>  dims = rhs.get_dims();
		create_higher_dims(dims, 0, datastore.data());		
	};

	cNDArray operator=(const cNDArray& rhs){
		_GSTITEM_
		datastore = rhs.datastore;
		std::vector<size_t>  dims = rhs.get_dims();		
		create_higher_dims(dims, 0, datastore.data());
		return *this;
	}

	void initialise(const std::vector<size_t>& dims, const size_t thisdim = 0, T* dataptr = NULL){
		_GSTITEM_
		if (thisdim == 0) allocate(dims);
		else pdata = dataptr;
		create_higher_dims(dims, thisdim, pdata);
	}

	size_t ndims() const {
		_GSTITEM_
		return 1 + nhigherdims();
	}

	size_t  nhigherdims() const {
		_GSTITEM_		
		return 1 + higher_dims[0].nhigherdims();
	}

	std::vector<size_t>  get_dims() const {
		_GSTITEM_		
		std::vector<size_t> hd     = hdims[0].get_dims();
		std::vector<size_t> dims  = { hdims.size() };
		dims.insert(dims.end(), hd.begin(), hd.end());
		return dims;
	}

	size_t size(){
		_GSTITEM_
		return hdims.size();
	}

	size_t nelements(){
		_GSTITEM_
		return size() * hdims[0].nelements();
	}

	T* data(){
		_GSTITEM_
		return pdata;
	}

	std::vector<T>& vector(){
		_GSTITEM_
		return datastore;
	}

	cNDArray<T, ND - 1>& operator[](const size_t& i){		
		#ifdef CNDARRAY_BOUNDSCHECK
		if (i < 0 || i >= hdims.size()){
			_GSTITEM_; std::printf("Subscript %lu is out of range (array size is %lu)\n", i, hdims.size());
			_GSTPRINT_; std::exception e("Subscript out of range exception\n");
			throw(e);
		}
		#endif // CNDARRAY_BOUNDSCHECK		
		return hdims[i];	
	}

	T& element(const size_t& i){
		_GSTITEM_
		#ifdef CNDARRAY_BOUNDSCHECK
		if (i < 0 || i >= nelements()){
			_GSTITEM_; std::printf("Subscript %lu is out of range (array size is %lu)\n", i, nelements());
			_GSTPRINT_; std::exception e("Subscript out of range exception\n");
			throw(e);
		}
		#endif // CNDARRAY_BOUNDSCHECK
		return pdata[i];
	}

	void printf(const char* fmt){
		_GSTITEM_
		for (size_t i = 0; i < size(); i++){
			higher_dims[i].printf(fmt);
		}
		std::printf("\n");		
	}

};

template<typename T>
class cNDArray<T, 1>{

private:
	size_t finaldimensionsize;
	T* pdata = (T*) NULL;
	
	bool create_higher_dims(const std::vector<size_t>& dims, const size_t& thisdim, T* dataptr){
		_GSTITEM_
		pdata = dataptr;
		finaldimensionsize = dims[thisdim];
		return true;
	}

public:

	cNDArray(){ _GSTITEM_ }
	
	cNDArray(const std::vector<size_t>& dims, const size_t thisdim = 0, T* dataptr = NULL){
		_GSTITEM_
		initialise(dims, thisdim, dataptr);
	}

	void initialise(const std::vector<size_t>& dims, const size_t thisdim = 0, T* dataptr = NULL){
		_GSTITEM_
		create_higher_dims(dims, thisdim, dataptr);
	}

	size_t ndims() const {
		_GSTITEM_
		return 1;
	}

	size_t  nhigherdims() const {
		_GSTITEM_
		return 0;
	}

	std::vector<size_t>  get_dims() const {
		std::vector<size_t> d(1);
		d[0] = size();
		return d;
	}

	size_t size() const {
		_GSTITEM_
		return finaldimensionsize;
	}

	size_t nelements(){
		_GSTITEM_
		return finaldimensionsize;
	}

	T& operator[](const size_t& i){
		_GSTITEM_
		#ifdef CNDARRAY_BOUNDSCHECK
			if (i < 0 || i >= finaldimensionsize){
				_GSTITEM_; std::printf("Subscript %lu is out of range (array size is %lu)\n", i, finaldimensionsize);
				_GSTPRINT_; std::exception e("Subscript out of range exception\n");
				throw(e);
			}
		#endif // CNDARRAY_BOUNDSCHECK
		return pdata[i];
	}

	T& element(const size_t& i){
		_GSTITEM_
		#ifdef CNDARRAY_BOUNDSCHECK
			if (i < 0 || i >= nelements()){
				_GSTITEM_; std::printf("Subscript %lu is out of range (array size is %lu)\n", i, nelements());
				_GSTPRINT_; std::exception e("Subscript out of range exception\n");
				throw(e);
		}
		#endif // CNDARRAY_BOUNDSCHECK
		return pdata[i];
	}

	void printf(const char* fmt){
		_GSTITEM_
		for (size_t i = 0; i < size(); i++){
			std::printf(fmt,pdata[i]);
		}
		std::printf("\n");		
		return;
	}

};

#endif

