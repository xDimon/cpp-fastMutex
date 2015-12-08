/*
 * MutexWatcher.hpp
 *
 *  Created on: 27 июля 2014 г.
 *      Author: Dmitriy Khaustov aka Dиmon (khaustov.dm@gmail.com)
 */

#ifndef XDIMON_UTILS_MUTEX_MUTEXWATCHER_HPP_
#define XDIMON_UTILS_MUTEX_MUTEXWATCHER_HPP_

#include <cstdint>

class Mutex;
class MutexInfo;

class MutexWatcher
{
public:
	bool set;
	MutexInfo *(*push)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n, bool captured);
	bool (*pop)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n);
};

#endif /* XDIMON_UTILS_MUTEX_MUTEXWATCHER_HPP_ */
