// Copyright (c) 2022, Thomas Janson
// All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#ifndef LIB_TOKEN_HPP_
#define LIB_TOKEN_HPP_

#include <HLS/hls.h>
#include <HLS/stdio.h>
#include <HLS/ac_int.h>
#include <HLS/ac_fixed.h>

// forward declaration
template<typename T, int A, int B>
class HLSVar;

template<typename T>
struct Token {
	T value {0};
	bool valid {false};
	constexpr Token<T>() : value {0}, valid {false} {};
	constexpr Token<T>(const T value_param,const bool valid_param) : value {value_param}, valid {valid_param} {};
	constexpr Token<T>(const T value_param) : value {value_param} , valid {true} {};
	operator T() const {
		return (*this).value;
	}
	template<typename S>
	Token<T> & operator=(const Token<S> & rhs) {
		(*this).valid = rhs.valid;
		(*this).value = rhs.value;
		return *this;
	}
	template<int A, int B>
	Token<T> & operator=(const HLSVar<T,A,B> & rhs) {
		(*this).valid = rhs.offset(0).valid;
		(*this).value = rhs.offset(0).value;
		return *this;
	}
};

template<typename T>
Token<T> operator+(const Token<T> & lhs, const Token<T> & rhs) {
	return {lhs.value + rhs.value, lhs.valid && rhs.valid};
};

template<typename T>
Token<T> operator-(const Token<T> & lhs, const Token<T> & rhs) {
	return {lhs.value - rhs.value, lhs.valid && rhs.valid};
};

template<typename T>
Token<T> operator*(const Token<T> & lhs, const Token<T> & rhs) {
	return {lhs.value * rhs.value, lhs.valid && rhs.valid};
};

template<typename T>
Token<T> operator/(const Token<T> & lhs, const Token<T> & rhs) {
	return {lhs.value / rhs.value, lhs.valid && rhs.valid};
}

template<typename T, typename S>
Token<T> operator/(const Token<T> & lhs, const Token<S> & rhs) {
	return {lhs.value / rhs.value, lhs.valid && rhs.valid};
};


#endif /* LIB_TOKEN_HPP_ */
