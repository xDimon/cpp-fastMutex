/*
 * MutexInfo.hpp
 *
 *  Created on: 27 июля 2014 г.
 *      Author: Dmitriy Khaustov aka Dиmon (khaustov.dm@gmail.com)
 */

#ifndef XDIMON_UTILS_MUTEX_MUTEXINFO_HPP_
#define XDIMON_UTILS_MUTEX_MUTEXINFO_HPP_

#include <sys/time.h>
#include <cstdint>

class Mutex;

class MutexInfo
{
public:
	const Mutex *mutex;
	const char *place;
	const uint16_t line;
	uint16_t count;
	timeval tv_try;
	timeval tv_done;
	MutexInfo(const Mutex *m, const char *p, uint16_t l, uint16_t c, bool captured): mutex(m), place(p), line(l), count(c)
	{
		gettimeofday(&tv_try, nullptr);
		if (captured)
		{
			tv_done = tv_try;
		}
		else
		{
			tv_done.tv_usec = tv_done.tv_sec = 0;
		}
	};
	void capture(uint16_t c)
	{
		gettimeofday(&tv_done, nullptr);
		count = c;
	}
};

#endif /* XDIMON_UTILS_MUTEX_MUTEXINFO_HPP_ */
