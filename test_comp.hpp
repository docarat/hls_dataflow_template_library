// Copyright (c) 2022, Thomas Janson
// All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#ifndef TEST_COMP_HPP_
#define TEST_COMP_HPP_

#include <HLS/hls.h>

#include <HLS/hls.h>
#include <HLS/stdio.h>
#include <HLS/math.h>
#include <HLS/ac_int.h>
#include <HLS/ac_fixed.h>
#include <HLS/hls_float.h>
#include <HLS/ac_complex.h>
#include <bitset>
#include <iostream>

#include "lib/HLSVar.hpp"

component Token<float> moving_avg_float(float stream_in);

component Token<uint10> moving_avg(uint10 stream_in);

component uint10 moving_avg_hls (uint10 stream_in);

component uint10 moving_avg_RTLMod(uint10 stream_in);

component uint10 convol2d(uint10 stream_in);

//component uint32_t myRTLMod(uint32_t stream_in);

component Token<int10> derivation(int10 stream_in);

using fixp_33_23 = ac_fixed<33,23,true>;
component Token<fixp_33_23> derivation_fixp(fixp_33_23 stream_in);

component float triangular_smooth_float(float stream_in);

component uint10 triangular_smooth_adc(uint10 stream_in);

component int11 peak_finder_adc(uint10 stream_in);

component Token<int> d_convol_comp(int psi_in, int u_in);

component int11 peak_finder_task_comp(uint10 stream_in);


#endif /* TEST_COMP_HPP_ */
