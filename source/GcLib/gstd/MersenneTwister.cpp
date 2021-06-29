#include "MersenneTwister.hpp"
#include <random>
using namespace gstd;

/**********************************************************
//MersenneTwister
//Mersenne Twisterは、松本眞 ・西村拓士（アルファベット順）
//により1996年から1997年に渡って開発された
//疑似乱数生成アルゴリズムです。
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/what-is-mt.html
**********************************************************/
//The original code was the Hiroshima code, it has been adjusted for C++11.
typedef unsigned long cardinal;
typedef long double real;
const cardinal MATRIX_A = 0x9908b0dfUL;
const cardinal UPPER_MASK = 0x80000000UL;
const cardinal LOWER_MASK = 0x7fffffffUL;

#define FIX32(value) value // 32bitの型が無い環境では value & 0xffffffffUL とか
static cardinal const mag01[2] = { 0x0UL, MATRIX_A };

MersenneTwister::MersenneTwister()
	: actualTwister(0)
	, seed(0)
{
}
MersenneTwister::MersenneTwister(unsigned long s)
	: actualTwister(s)
	, seed(s)
{
}
unsigned long MersenneTwister::_GenrandInt32()
{
	std::uniform_int_distribution<long> new_param(0, maxInt);
	return new_param(actualTwister);
}
void MersenneTwister::Initialize(unsigned long s)
{
	actualTwister.seed(s);
	seed = s;
}
void MersenneTwister::Initialize(unsigned long* init_key, int key_length)
{
	Initialize(0);
}
int MersenneTwister::SeedRNG()
{
	long result;
	std::uniform_int_distribution<int> new_param(0, maxInt);
	result = new_param(actualTwister);
	Initialize(result);
	return result;
}
long MersenneTwister::GetInt()
{
	std::uniform_int_distribution<int> new_param(0, maxInt);
	return new_param(actualTwister);
}
long MersenneTwister::GetInt(long min, long max)
{
	std::uniform_int_distribution<int> new_param(min, max);
	return new_param(actualTwister);
}
int64_t MersenneTwister::GetInt64()
{
	std::uniform_int_distribution<int64_t> new_param(0, maxInt64);
	return new_param(actualTwister);
}
int64_t MersenneTwister::GetInt64(int64_t min, int64_t max)
{
	std::uniform_int_distribution<int64_t> new_param(min, max);
	return new_param(actualTwister);
}
long double MersenneTwister::GetReal()
{
	std::uniform_real_distribution<long double> new_param(0, maxReal);
	return new_param(actualTwister);
}
long double MersenneTwister::GetReal(long double min, long double max)
{
	std::uniform_real_distribution<long double> new_param(min, max);
	return new_param(actualTwister);
}