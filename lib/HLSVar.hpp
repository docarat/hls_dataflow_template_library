// Copyright (c) 2022, Thomas Janson
// All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#ifndef LIB_HLSVAR_HPP_
#define LIB_HLSVAR_HPP_

#include <HLS/hls.h>
#include <HLS/stdio.h>
#include <HLS/ac_int.h>
#include <HLS/ac_fixed.h>
#include "Token.hpp"

template<typename T, int MAX_OFFSET=0, int MIN_OFFSET=0>
class HLSVar {
private:
	constexpr static int maximal_offset = (MAX_OFFSET<0) ? 0 : MAX_OFFSET;
	constexpr static int minimal_offset = (MIN_OFFSET>0) ? 0 : (-1)*MIN_OFFSET;
	constexpr static int depth_of_pipeline = minimal_offset + maximal_offset;
	constexpr static int pipeline_depth = (depth_of_pipeline==0) ? 1  : depth_of_pipeline+1;
	constexpr static int ancor_point = depth_of_pipeline - maximal_offset;

	hls_register Token<T> pipeline[pipeline_depth];

	Token<T> operator()(T input_val) {
		#pragma unroll
		for (int i = 0; i < pipeline_depth-1; ++i) {
			pipeline[i] = pipeline[i+1];
		}
		pipeline[pipeline_depth-1] = {input_val,true};
		return pipeline[0];
	}

	Token<T> operator()(Token<T> input_val) {
		if (input_val.valid) {
			#pragma unroll
			for (int i = 0; i < pipeline_depth-1; ++i) {
				pipeline[i] = pipeline[i+1];
			}
			pipeline[pipeline_depth-1] = {input_val.value,input_val.valid};
		}
		return pipeline[0];
	}

public:

	auto operator=(const T & rhs) {
		return (*this)(rhs);
	}

	template<typename S,int A,int B>
	auto operator=(const HLSVar<S,A,B> & rhs) {
		return (*this)({rhs.offset(0).value,rhs.offset(0).valid});
	}

	template<typename S>
	auto operator=(const Token<S> & rhs) {
		return (*this)({rhs.value,rhs.valid});
	}

	Token<T> offset(int offset_val) const {
		return pipeline[offset_val+ancor_point];
	}

	operator T() const {
		return (*this).offset(0);
	}

};

template<typename T, int A, int B, int C, int D>
auto operator+(HLSVar<T,A,B> &lhs,HLSVar<T,C,D> &rhs) {
    return Token<T>{lhs.offset(0).value + rhs.offset(0).value,lhs.offset(0).valid && rhs.offset(0).valid};
}

template<typename T, int A, int B, int C, int D>
auto operator-(const HLSVar<T,A,B> & lhs,const HLSVar<T,C,D> & rhs) {
	return Token<T>{lhs.offset(0).value - rhs.offset(0).value,lhs.offset(0).valid && rhs.offset(0).valid};
}

template<typename T, int A, int B, int C, int D>
auto operator*(HLSVar<T,A,B> &lhs,HLSVar<T,C,D> &rhs) {
	return Token<T>{lhs.offset(0).value * rhs.offset(0).value,lhs.offset(0).valid && rhs.offset(0).valid};
}

template<typename T, int A, int B, int C, int D>
auto operator/(HLSVar<T,A,B> &lhs,HLSVar<T,C,D> &rhs) {
	return Token<T>{lhs.offset(0).value / rhs.offset(0).value,lhs.offset(0).valid && rhs.offset(0).valid};
}

#endif /* LIB_HLSVAR_HPP_ */
