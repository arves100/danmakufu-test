#ifndef __GSTD_MERSENNETWISTER__
#define __GSTD_MERSENNETWISTER__

#include"GstdConstant.hpp"
#include<random>
namespace gstd {

/**********************************************************
//MersenneTwister
//Mersenne Twisterは、松本眞 ・西村拓士（アルファベット順）
//により1996年から1997年に渡って開発された
//疑似乱数生成アルゴリズムです。
// http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/what-is-mt.html
**********************************************************/
class MersenneTwister {
	struct mt_struct;
	friend struct mt_struct;
	enum {
		MT_N = 624,
		MT_M = 397,
	};

	typedef struct mt_struct {
		unsigned long mt[MT_N];
		int mti;
		virtual ~mt_struct() {}
	} mt_struct;
	mt_struct mts;
	unsigned long _GenrandInt32();

private:
	std::mt19937 actualTwister;
	int seed;
	long double maxReal = 3.402822e+38;
	long maxInt = 2147483647;
	unsigned long long maxInt64 = 9223372036854775807;

public:
	MersenneTwister();
	MersenneTwister(unsigned long s);
	virtual ~MersenneTwister() {}
	void Initialize(unsigned long s);
	void Initialize(unsigned long* init_key, int key_length);

	int GetSeed() { return seed; }
	int SeedRNG();
	long GetInt();
	long GetInt(long min, long max);
	int64_t GetInt64();
	int64_t GetInt64(int64_t min, int64_t max);
	long double GetReal();
	long double GetReal(long double min, long double max);
};

} // namespace gstd

#endif
