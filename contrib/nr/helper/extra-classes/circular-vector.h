/*
 * circular-vector.h
 *
 *  Created on: Jan 24, 2022
 *      Author: ugjeci
 */

#include <ostream>

#ifndef SL_CONTRIB_HELPER_CIRCULAR_VECTOR_H_
#define SL_CONTRIB_HELPER_CIRCULAR_VECTOR_H_


namespace ns3 {
	//#define NUM_ELTS 100
	template < typename T >
	class CircularVector{
	public:
	   CircularVector(int size) : idx(0), size(size){
		   vec = std::vector<T>(this->size);
	   }
	   void push_back(T& elt)
	   {
		  vec[ idx++ % this->size ] = elt;
	   }
	private:
	   int idx;
	   std::vector<T> vec;
	   int size;
	};

}




#endif /* CONTRIB_ELEPHANT_FLOW_HELPER_CIRCULAR_VECTOR_H_ */
