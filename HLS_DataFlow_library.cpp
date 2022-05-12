// Copyright (c) 2022, Thomas Janson
// All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HLS_DataFlow_library_test

#include <boost/test/unit_test.hpp>

#include <HLS/hls.h>
#include <HLS/stdio.h>
#include <HLS/math.h>
#include <HLS/ac_int.h>
#include <HLS/ac_fixed.h>
#include <HLS/hls_float.h>
#include <HLS/ac_complex.h>
#include <bitset>
#include <iostream>
#include <fstream>
#include <string>

#include "lib/HLSVar.hpp"
#include "test_comp.hpp"

BOOST_AUTO_TEST_SUITE(HLSVar_test)

BOOST_AUTO_TEST_CASE(assignment_and_type_conversion_operator_001)
{
	uint10 test_value {1023};
	HLSVar<uint10> stream;
	stream = test_value;
	uint10 result = stream;
	BOOST_REQUIRE_EQUAL(test_value,result);
}

BOOST_AUTO_TEST_CASE(assignemnt_operator_test_for_HLSVar_to_HLSVar_002)
{
	uint10 test_value {1023};
	HLSVar<uint10> stream_in;
	HLSVar<uint10> stream_out;
	stream_in = test_value;
	stream_out = stream_in;
	uint10 result = stream_out;
	BOOST_REQUIRE_EQUAL(test_value,result);
}

BOOST_AUTO_TEST_CASE(offset_operator_and_shift_register_test_003)
{
	uint10 test_stream[3] {1,2,3};
	HLSVar<uint10,1,-1> stream;
	stream = test_stream[0];
	stream = test_stream[1];
	stream = test_stream[2];
	uint10 result[3];
	result[0] = stream.offset(-1);
	result[1] = stream.offset(0);
	result[2] = stream.offset(+1);
	BOOST_REQUIRE_EQUAL(test_stream[0],result[0]);
	BOOST_REQUIRE_EQUAL(test_stream[1],result[1]);
	BOOST_REQUIRE_EQUAL(test_stream[2],result[2]);
}

BOOST_AUTO_TEST_CASE(assignment_operator_stream_into_stream_test_004)
{
	uint10 test_stream[3] {1,2,3};
	uint10 result[3];
	HLSVar<uint10,1,-1> stream_in;
	HLSVar<uint10,2,-2> stream_out;

	int tick {0};
	for (int i = 0; i<8; ++i) {
		stream_in = (i<3) ? test_stream[i] : static_cast<uint10>(0);
		stream_out = stream_in;
		Token<uint10> readval = stream_out.offset(0);
		if (readval.valid && (tick<3)) {
			result[tick] = readval.value;
			tick++;
		}
	}
	for (int i=0; i<3; ++i) {
		BOOST_REQUIRE_EQUAL(test_stream[i],result[i]);
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Token_test)

BOOST_AUTO_TEST_CASE(token_constructor)
{
	Token<int10> instanz_var_default;
	BOOST_REQUIRE_EQUAL(instanz_var_default.value,0);
	BOOST_REQUIRE_EQUAL(instanz_var_default.valid,0);
	Token<int10> instanz_var_full {511,1};
	BOOST_REQUIRE_EQUAL(instanz_var_full.value,511);
	BOOST_REQUIRE_EQUAL(instanz_var_full.valid,1);
	Token<int10> instanz_var_value {0};
	BOOST_REQUIRE_EQUAL(instanz_var_value.value,0);
	BOOST_REQUIRE_EQUAL(instanz_var_full.valid,1);
}

BOOST_AUTO_TEST_CASE(token_assignment_same_type)
{
	Token<uint10> test_input {263};
	Token<uint10> test_result;
	test_result = test_input; // Token<uint10> = Token<uint10>
	BOOST_REQUIRE_EQUAL(test_result.value,test_input.value);
	BOOST_REQUIRE_EQUAL(test_result.valid,true);

	uint10 integer_result;
	integer_result = test_input;  // uint10 = Token<uint10>
	BOOST_REQUIRE_EQUAL(test_input.value,integer_result);

	uint10 test_integer {1023};
	Token<uint10> result_token;
	result_token = test_integer; //Token<uint10> = uint10
	BOOST_REQUIRE_EQUAL(result_token.value,test_integer);
	BOOST_REQUIRE_EQUAL(result_token.valid,true);
}

BOOST_AUTO_TEST_CASE(token_assignment_type_conversion)
{
	int test_val {512};
	int10 ac_int_test_val;
	ac_int_test_val = test_val; // C semantic allows that type conversion
	                            // but in the 10 bit integer, the sign bit is set now !
	BOOST_REQUIRE_EQUAL(ac_int_test_val,-512);

	uint10 ac_uint_test_val = test_val;
	BOOST_REQUIRE_EQUAL(ac_uint_test_val,512);

	test_val = 1024;
	ac_uint_test_val = test_val;
	BOOST_REQUIRE_EQUAL(ac_uint_test_val,0);

	Token<int> test_value_int {1055};
	Token<float> converted_into_float;
	converted_into_float = test_value_int;
	BOOST_TEST(converted_into_float.value == test_value_int.value,boost::test_tools::tolerance(0.0001)); // @suppress("Method cannot be resolved")
	BOOST_REQUIRE_EQUAL(converted_into_float.valid, test_value_int.valid);

	float converted_float_value = converted_into_float;
	BOOST_TEST(converted_float_value == test_value_int.value,boost::test_tools::tolerance(0.0001)); // @suppress("Method cannot be resolved")

	Token<float> test_value_float {2365.32};
	Token<int> converted_into_int;
	converted_into_int = test_value_float; // narrowing conversion allowed in C
	BOOST_TEST(converted_into_int.value == (test_value_float.value-0.32),boost::test_tools::tolerance(0.0001)); // @suppress("Method cannot be resolved")
	BOOST_REQUIRE_EQUAL(converted_into_int.valid, test_value_float.valid);

	int converted_int_value = converted_into_int;
	BOOST_TEST(converted_int_value == (test_value_float.value-0.32),boost::test_tools::tolerance(0.0001)); // @suppress("Method cannot be resolved")
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(moving_average_filter)



BOOST_AUTO_TEST_CASE(single_invocation_hls_test)
{
	constexpr int SIZE = 20;
	uint10 stream_in[SIZE] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
	uint10 stream_out[SIZE] {0};

	uint10 golden_result[SIZE] {0};
	for (int i = 0; i < SIZE ;++i) {
		uint10 prev = i>0 ? static_cast<uint10>(stream_in[i-1]) : static_cast<uint10>(0);
		uint10 pres = static_cast<uint10>(stream_in[i]);
		uint19 next = (i+1)<SIZE ? static_cast<uint10>(stream_in[i+1]) : static_cast<uint10>(0);
		golden_result[i+1] = (prev+pres+next)/3;
	}

	for (int i=0; i<SIZE; ++i) {
		stream_out[i] = moving_avg_hls(stream_in[i]);
	}
	std::cout << std::endl;

	for (int i=0; i <SIZE; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],stream_out[i]);
	}

	moving_avg_hls(0); moving_avg_hls(0);moving_avg_hls(0); //TODO: flush the stream
}

BOOST_AUTO_TEST_CASE(streaming_float_test)
{
	constexpr int SIZE = 20;
	float stream_in[SIZE] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
	float stream_out[SIZE] {0};

	float golden_result[SIZE] {0};
	for (int i = 0; i < SIZE ;++i) {
		float prev = i>0 ? static_cast<float>(stream_in[i-1]) : static_cast<float>(0);
		float pres = static_cast<float>(stream_in[i]);
		float next = (i+1)<SIZE ? static_cast<float>(stream_in[i+1]) : static_cast<float>(0);
		golden_result[i+1] = (prev+pres+next)/3.0;
	}

	for (int i = 0; i<SIZE; ++i) {
		ihc_hls_enqueue(&stream_out[i],&moving_avg_float,stream_in[i]);
	}
	ihc_hls_component_run_all(moving_avg_float);

	for (int i=0; i <SIZE; ++i) {
    		BOOST_REQUIRE_CLOSE(golden_result[i]-stream_out[i],0.0,1e-8);
	}
}

BOOST_AUTO_TEST_CASE(streaming_hls_test)
{
	constexpr int SIZE = 20;
	uint10 stream_in[SIZE] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
	uint10 stream_out[SIZE] {0};

	uint10 golden_result[SIZE] {0};
	for (int i = 0; i < SIZE ;++i) {
		uint10 prev = i>0 ? static_cast<uint10>(stream_in[i-1]) : static_cast<uint10>(0);
		uint10 pres = static_cast<uint10>(stream_in[i]);
		uint19 next = (i+1)<SIZE ? static_cast<uint10>(stream_in[i+1]) : static_cast<uint10>(0);
		golden_result[i+1] = (prev+pres+next)/3;
	}

	for (int i = 0; i<SIZE; ++i) {
		ihc_hls_enqueue(&stream_out[i],&moving_avg_hls,stream_in[i]);
	}
	ihc_hls_component_run_all(moving_avg_hls);

	for (int i=0; i <SIZE; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],stream_out[i]);
	}
}

BOOST_AUTO_TEST_CASE(moving_average_div_operator)
{
	constexpr int SIZE = 20;
	uint10 stream_in[SIZE] {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
	Token<uint10> stream_out[SIZE];

	uint10 golden_result[SIZE] {0};
	for (int i = 0; i < SIZE ;++i) {
		uint10 prev = i>0 ? static_cast<uint10>(stream_in[i-1]) : static_cast<uint10>(0);
		uint10 pres = static_cast<uint10>(stream_in[i]);
		uint19 next = (i+1)<SIZE ? static_cast<uint10>(stream_in[i+1]) : static_cast<uint10>(0);
		golden_result[i+1] = (prev+pres+next)/3;
	}

	for (int i=0; i<SIZE; ++i) {
		stream_out[i] = moving_avg(stream_in[i]);
	}
	std::cout << std::endl;

	for (int i=0; i <SIZE; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],stream_out[i].value);
	}

	moving_avg(0); moving_avg(0);moving_avg(0); //TODO: flush the stream
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(convol2D_test)

BOOST_AUTO_TEST_CASE(single_invocation)
{
	constexpr int COL {5};
	constexpr int ROW {5};

	uint10 array_in [ROW*COL] {0}; // MxN Array
	for (int i = 0; i<(ROW*COL); ++i) {
		array_in[i] = i;
	}

	uint10 array_out [ROW*COL] {0};
	for (int i = 0; i<ROW; ++i) {
		convol2d(array_in[i]);   // filling pipeline
	}
	for (int i = 0; i<(ROW*COL-ROW); ++i) {
		array_out[i] = convol2d(array_in[i+ROW]);
	}
	for (int i = ROW*COL-ROW; i<ROW*COL; ++i) {
		array_out[i] = convol2d(static_cast<uint10>(0)); //flushing pipeline
	}
//	for (int i = 0; i<(ROW*COL); ++i) {
//		std::cout << " " << array_out[i];
//	}
//	std::cout << std::endl;

	uint10 golden_result [ROW*COL] {6,9,13,17,16,21,30,35,40,35,41,55,60,65,55,61,80,85,90,75,56,79,83,87,66};
	for (int i = 0; i<ROW*COL; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],array_out[i]);
	}
}

BOOST_AUTO_TEST_CASE(streaming_test)
{
	constexpr int COL {5};
	constexpr int ROW {5};

	uint10 array_in [ROW*COL] {0}; // MxN Array
	for (int i = 0; i<(ROW*COL); ++i) {
		array_in[i] = i;
	}

	uint10 array_out [ROW*COL] {0};

	for (int i = 0; i<ROW; ++i) {
		ihc_hls_enqueue(&array_out[0],&convol2d,array_in[i]); // filling pipeline
	}

	for (int i = 0; i<(ROW*COL-ROW); ++i) {
		ihc_hls_enqueue(&array_out[i],&convol2d,array_in[i+ROW]); // pipeline filled
	}

	for (int i = ROW*COL-ROW; i<ROW*COL; ++i) {
		ihc_hls_enqueue(&array_out[i],&convol2d,static_cast<uint10>(0)); //flushing pipeline
	}

	ihc_hls_component_run_all(convol2d);

	uint10 golden_result [ROW*COL] {6,9,13,17,16,21,30,35,40,35,41,55,60,65,55,61,80,85,90,75,56,79,83,87,66};
	for (int i = 0; i<ROW*COL; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],array_out[i]);
	}
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(peak_finder_test)

BOOST_AUTO_TEST_CASE(derivation_component)
{
	constexpr int SIZE = 20;
	int10 stream_in[SIZE] {0,1,2,3,4,2,5,7,8,7,5,2,3,1,0,1,2,1,2,2};
	int10 golden_result[SIZE] {1,1,0,0,1,2,1,-1,-2,-1,-1,-1,-1,+1,0,0,0,0,-1,0};
	Token<int10> stream_out[SIZE]; // {0};

	int stream_out_count {0};
	int stream_in_count {0};
	Token<int10> null; // {0};
	do {
		stream_out[stream_out_count] = (stream_in_count<20) ? derivation(stream_in[stream_in_count]) : derivation(null);
		++stream_in_count;
		if (stream_out[stream_out_count].valid) {
			++stream_out_count;
		}
	}while(stream_out_count != 20);

	for (int i=0; i<SIZE; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],stream_out[i].value);
	}
}

BOOST_AUTO_TEST_CASE(derivation_fixpoint_component)
{
	constexpr int SIZE = 20;
	fixp_33_23 stream_in[SIZE] {0,1,2,3,4,2,5,7,8,7,5,2,3,1,0,1,2,1,2,2};
	Token<fixp_33_23> stream_out[SIZE];

	fixp_33_23 avg[SIZE] {0};
	for (int i=0; i<SIZE; ++i) {
		fixp_33_23 sum01 = stream_in[i];
		fixp_33_23 sum02 = ( (i+1) < SIZE ) ? stream_in[i+1] : 0;
		fixp_33_23 sum03 = ( (i+2) < SIZE ) ? stream_in[i+2] : 0;
		constexpr fixp_33_23 one_third {1.0/3.0};
		fixp_33_23 sum; sum = sum01 + sum02 + sum03;
		avg[i] = (sum01 + sum02 + sum03) * one_third;
	}
	fixp_33_23 golden_result[SIZE] {0};
	for (int i=0; i<SIZE; ++i) {
		fixp_33_23 diff01 = avg[i];
		fixp_33_23 diff02 = ( (i+1) < 20) ? avg[i+1] : 0;
		golden_result[i] = diff02 - diff01;
	}

	int stream_out_count {0};
	int stream_in_count {0};
	Token<fixp_33_23> null {0};
	do {
		stream_out[stream_out_count] = (stream_in_count<20) ? derivation_fixp(stream_in[stream_in_count]) : derivation_fixp(null);
		++stream_in_count;
		if (stream_out[stream_out_count].valid) {
			++stream_out_count;
		}
	}while(stream_out_count != 20);

	for (int i=0; i<SIZE; ++i) {
		BOOST_REQUIRE_EQUAL(golden_result[i],stream_out[i].value);
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(file_IO_test)

//#include <iostream>
//#include <fstream>
//#include <string>

BOOST_AUTO_TEST_CASE(read_comma_seperated_file)
{
	std::vector<float> test_data;
	std::ifstream file("data/data.dat");
	std::string data;
	while(std::getline(file,data,','))
	{
		float fl = std::stod(data);
		test_data.push_back(fl);
		//std::cout << fl << std::endl;
	}
	//std::cout << "Number of data items = " << test_data.size() << std::endl;
	file.close();
}

BOOST_AUTO_TEST_CASE(write_result_to_file)
{
	std::vector<float> test_data;
	std::ofstream file("result/result.dat");
	float test_val = 2.0;
	for (int i=0; i<10; ++i) {
		std::string out = std::to_string(test_val*i);
		file << out << std::endl;
	}
	file.close();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(peak_finder_algorithm)

BOOST_AUTO_TEST_CASE(triangular_seven_point_smooth_float)
{
	std::vector<float> test_data;
	std::ifstream input_file("data/data.dat");
	std::string data;
	while(std::getline(input_file,data,','))
	{
		float fl = std::stod(data);
		test_data.push_back(fl);
	}
	input_file.close();

	std::vector<float> result (test_data.size(),0.0);
	for (int i=0; i<test_data.size(); ++i) {
		ihc_hls_enqueue(&result[i],&triangular_smooth_float,test_data[i]);
	}
	ihc_hls_component_run_all(triangular_smooth_float);

	std::ofstream output_file("result/result01.dat");
	for (int i=0; i<result.size() ;++i) {
		output_file << result[i] << std::endl;
	}
	output_file.close();
}

BOOST_AUTO_TEST_CASE(triangular_seven_point_smooth_uint10)
{
	std::vector<uint10> test_data;
	std::ifstream input_file("data/data.dat");
	std::string data;
	while(std::getline(input_file,data,','))
	{
		float val = std::stod(data);
		test_data.push_back(static_cast<uint10>(val));
	}
	input_file.close();

	std::vector<uint10> result (test_data.size(),0.0);
	for (int i=0; i<test_data.size(); ++i) {
		ihc_hls_enqueue(&result[i],&triangular_smooth_adc,test_data[i]);
	}
	ihc_hls_component_run_all(triangular_smooth_adc);

	std::ofstream test_data_file("result/test_data.dat");
	std::ofstream output_file("result/result02.dat");
	for (int i=0; i<result.size() ;++i) {
		output_file << result[i] << std::endl;
		test_data_file << test_data[i] << std::endl;
	}
	output_file.close();
	test_data_file.close();
}

BOOST_AUTO_TEST_CASE(peak_finder_test)
{
	std::vector<uint10> test_data;
	std::ifstream input_file("data/data.dat");
	std::string data;
	while(std::getline(input_file,data,','))
	{
		float val = std::stod(data);
		test_data.push_back(static_cast<uint10>(val));
	}
	input_file.close();

	std::vector<int11> result (test_data.size(),0.0);
	for (int i=0; i<test_data.size(); ++i) {
		ihc_hls_enqueue(&result[i],&peak_finder_adc,test_data[i]);
	}
	ihc_hls_component_run_all(peak_finder_adc);

	std::ofstream output_file("result/derivative.dat");
	for (int i=0; i<result.size() ;++i) {
		output_file << result[i] << std::endl;
	}
	output_file.close();
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(graph_balancing)

BOOST_AUTO_TEST_CASE(test_comp_test)
{
	constexpr int N = 3;

	std::vector<int> psi(N*N);
	std::vector<int> u(N*N);

	for( int i = 0; i<(N*N); i++) {
		u[i] = 1; //;N*N-i;
		psi[i] = i+1;
	}

	std::vector<int> goldenResult(N*N);

	for (int i=0; i<N*N; ++i) {
		if ( i < (N*(N-1)) ) {
			goldenResult[i] = psi[i+N] * u[i] + psi[i];
		} else {
			goldenResult[i] = psi[i-(N*(N-1))] * u[i] + psi[i];
		}
	}

	std::vector<int> result(N*N);

	int j = 0;
	int i = 0;
	do {
		Token<int> resultToken;
		if (i<N*N) {
			resultToken = d_convol_comp(psi[i],u[i]);
			i++;
		} else {
			resultToken = d_convol_comp(0,0);
			i++;
		}
		if (resultToken.valid) {
			result[j] = resultToken.value;
			j++;
		}
	} while (j<N*N);

	for (int i = 0; i<(N*N); i++) {
		BOOST_REQUIRE_EQUAL(goldenResult[i],result[i]);
	}
}

BOOST_AUTO_TEST_CASE(peak_finder_task_test)
{
	std::vector<uint10> test_data;
	std::ifstream input_file("data/data.dat");
	std::string data;
	while(std::getline(input_file,data,','))
	{
		float val = std::stod(data);
		test_data.push_back(static_cast<uint10>(val));
	}
	input_file.close();

	std::vector<int11> result (test_data.size(),0.0);
	for (int i=0; i<test_data.size(); ++i) {
		ihc_hls_enqueue(&result[i],&peak_finder_task_comp,test_data[i]);
	}
	ihc_hls_component_run_all(peak_finder_task_comp);

	std::ofstream output_file("result/derivative_task.dat");
	for (int i=0; i<result.size() ;++i) {
		output_file << result[i] << std::endl;
	}
	output_file.close();
}

BOOST_AUTO_TEST_SUITE_END()
