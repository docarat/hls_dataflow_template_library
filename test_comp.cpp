// Copyright (c) 2022, Thomas Janson
// All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include "test_comp.hpp"


component Token<float> moving_avg_float(float stream_in) {
	static HLSVar<float,1,-1> stream;
		stream = stream_in;
		constexpr float rezipthree {1.0/3.0};
		const Token<float> rez_three {rezipthree,true};
		Token<float> avg = (stream.offset(-1) + stream.offset(0) + stream.offset(+1)) * rez_three;
		return avg;
}

component Token<uint10> moving_avg(uint10 stream_in) {
	static HLSVar<uint10,1,-1> stream;
	stream = stream_in;
	const Token<uint10> three {3,true};
	Token<uint10> avg = (stream.offset(-1) + stream.offset(0) + stream.offset(+1)) / three;
	return avg;
}

component uint10 moving_avg_hls (uint10 stream_in) {
	static HLSVar<uint10, 1,-1> stream;
	constexpr ac_fixed<33,10,false> rezipthree {1.0/3.0};
	stream = stream_in;
	ac_fixed<33,10,false> res = (stream.offset(-1) + stream.offset(0) + stream.offset(1)).value * rezipthree;
	uint10 round_off_result = res.slc<10>(23);
	uint10 round_up_result = round_off_result + 1;
	uint10 result = res[22] ? round_up_result : round_off_result;
	return result;
}

//component uint10 moving_avg_RTLMod(uint10 stream_in) {
//	return moving_avg_rtl(stream_in);
//}

component uint10 convol2d (uint10 stream_in) {
	constexpr int COL {5};
	static HLSVar<uint10, COL, -COL> stream;
	stream = stream_in;

	static uint3 colCount {1};

	const Token<uint10> zero {0,true};

	Token<uint10> prev10 = stream.offset(-COL);
	Token<uint10> prev01 = (colCount!=1) ? stream.offset(-1) : zero;
	Token<uint10> presnt = stream.offset(0);
	Token<uint10> next01 = (colCount!=5) ? stream.offset(1) : zero;
	Token<uint10> next10 = stream.offset(COL);

	Token<uint10> result;
	result = (prev10+prev01+presnt+next01+next10);

	if (colCount==5) { colCount=1; } else {++colCount;}

	return result.value;
}

//component uint32_t myRTLMod(uint32_t stream_in) {
//	return myMod(stream_in);
//}

component Token<int10> derivation(int10 stream_in) {
	static HLSVar<int10,1,-1> input_stream;
	input_stream = stream_in;
	constexpr Token<int10> three {3,true};
	Token<int10> avg = (input_stream.offset(-1) + input_stream.offset(0) + input_stream.offset(+1)) / three;
	static HLSVar<int10,1,0> diff_stream;
	diff_stream = avg;
	Token<int10> diff = diff_stream.offset(1) - diff_stream.offset(0);
	return diff;
}

component Token<fixp_33_23> derivation_fixp(fixp_33_23 stream_in) {
	static HLSVar<fixp_33_23,1,-1> input_stream;
	input_stream = stream_in;
	constexpr Token<fixp_33_23> one_third {1.0/3.0,true};
	Token<fixp_33_23> avg = (input_stream.offset(-1) + input_stream.offset(0) + input_stream.offset(+1)) * one_third;
	static HLSVar<fixp_33_23,1,0> diff_stream;
	diff_stream = avg;
	Token<fixp_33_23> diff = diff_stream.offset(1) - diff_stream.offset(0);
	return diff;
}

component float triangular_smooth_float(float stream_in)
{
	static HLSVar<float,3,-3> stream;
	stream = stream_in;
	constexpr float coeff_value {1.0/16.0};
	const Token<float> coeff {coeff_value,true};
	Token<float> smoothed = (stream.offset(-3) + Token<float>(2.0)*stream.offset(-2) + Token<float>(3.0)*stream.offset(-1)
			+ Token<float>(4.0)*stream.offset(0)
			+ Token<float>(3.0)*stream.offset(+1) + Token<float>(2.0)*stream.offset(+2) + stream.offset(+3)) * coeff;
	return smoothed.value;
}

component uint10 triangular_smooth_adc(uint10 stream_in)
{
	static HLSVar<uint14,3,-3> stream;
	stream = stream_in;
	uint14 smoothed = (stream.offset(-3) + Token<uint14>(2)*stream.offset(-2) + Token<uint14>(3)*stream.offset(-1)
			+ Token<uint14>(4)*stream.offset(0)
			+ Token<uint14>(3)*stream.offset(+1) + Token<uint14>(2)*stream.offset(+2) + stream.offset(+3)).value;
	uint10 result = smoothed >> 4;
	return result;
}

component int11 peak_finder_adc(uint10 stream_in)
{
	static HLSVar<uint14,3,-3> triangular_stream_buffer;
	triangular_stream_buffer = stream_in;
	static HLSVar<uint14,1,-1> smoothed_stream;
	smoothed_stream = (triangular_stream_buffer.offset(-3) + Token<uint14>(2)*triangular_stream_buffer.offset(-2)
			+ Token<uint14>(3)*triangular_stream_buffer.offset(-1) + Token<uint14>(4)*triangular_stream_buffer.offset(0)
			+ Token<uint14>(3)*triangular_stream_buffer.offset(+1) + Token<uint14>(2)*triangular_stream_buffer.offset(+2)
			+ triangular_stream_buffer.offset(+3))/Token<uint14>(16);
	static HLSVar<int11> derivative;
	derivative = ( smoothed_stream.offset(-1) - smoothed_stream.offset(1) ) / Token<int2>(2);
	int11 result = derivative.offset(0).value;
	return result;
}

component Token<int> d_convol_comp(int psi_in, int u_in)
{
        constexpr int N = 3;

        static int psi_input_stream_counter {0};

        static HLSVar<int,N,-(N*(N-1))> psi_stream_buffer;
        psi_stream_buffer = psi_in;

        static HLSVar<int,N> u_stream_buffer;
        u_stream_buffer = u_in;

        static HLSVar<int> psi_multiplexer_out;
        if (psi_input_stream_counter < (N*(N-1))) {
                psi_multiplexer_out = psi_stream_buffer.offset(N);
        } else {
                psi_multiplexer_out = psi_stream_buffer.offset(-N*(N-1));
        }

        static HLSVar<int> matrics_vector_prod;
        matrics_vector_prod = psi_multiplexer_out * u_stream_buffer;

        static HLSVar<int> result_buffer;
        result_buffer = psi_stream_buffer + matrics_vector_prod;

        Token<int> result;
        result = result_buffer;

        if (psi_input_stream_counter == N*N) {
                psi_input_stream_counter = 0;
        }else if(psi_stream_buffer.offset(0).valid) {
                psi_input_stream_counter++;
        }

        return result;
}

int11 peak_finder_task_function(uint10 stream_in)
{
	static HLSVar<uint14,3,-3> triangular_stream_buffer;
	triangular_stream_buffer = stream_in;
	static HLSVar<uint14,1,-1> smoothed_stream;
	smoothed_stream = (triangular_stream_buffer.offset(-3) + Token<uint14>(2)*triangular_stream_buffer.offset(-2)
			+ Token<uint14>(3)*triangular_stream_buffer.offset(-1) + Token<uint14>(4)*triangular_stream_buffer.offset(0)
			+ Token<uint14>(3)*triangular_stream_buffer.offset(+1) + Token<uint14>(2)*triangular_stream_buffer.offset(+2)
			+ triangular_stream_buffer.offset(+3)); // /Token<uint14>(16);
	//static HLSVar<int11> derivative;
	//derivative = ( smoothed_stream.offset(-1) - smoothed_stream.offset(1) ) /  Token<int2>(2);
	int15 derivative = ( smoothed_stream.offset(1).value - smoothed_stream.offset(-1).value );
	int11 result = derivative >> 5;
	return result;
}

component int11 peak_finder_task_comp(uint10 stream_in)
{
	int11 result;
	ihc::launch<peak_finder_task_function>(stream_in);
	result = ihc::collect<peak_finder_task_function>();
	return result;
}

