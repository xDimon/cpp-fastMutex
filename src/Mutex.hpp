/*
 * Mutex.hpp
 *
 *  Created on: 22 июля 2014 г.
 *      Author: Dmitriy Khaustov aka Dиmon (khaustov.dm@gmail.com)
 */

#ifndef XDIMON_UTILS_MUTEX_MUTEX_HPP_
#define XDIMON_UTILS_MUTEX_MUTEX_HPP_

#include <pthread.h>
#include <cstdint>

class MutexInfo;
class MutexWatcher;

/**
 * Мьютекс (взаимное исключение)
 */
class Mutex
{
private:
	static MutexWatcher _watcher;

public:
	static bool setWatcher(
		MutexInfo *(*push)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n, bool captured),
		bool (*pop)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n)
	);

private:
	const char *_name;
	bool _mutex;
	pthread_t _owner;
	int _count;

	int _lock(const char *place, int line);
	int _trylock(const char *place, int line);
	int _unlock(const char *place, int line);

public:
	Mutex(const char *name);
	virtual ~Mutex();

	const char *getName() const
	{
		return _name;
	}

	void lock();
	bool trylock();
	void unlock();

	void lock(const char *place, int line);
	bool trylock(const char *place, int line);
	void unlock(const char *place, int line);
};

#endif /* XDIMON_UTILS_MUTEX_MUTEX_HPP_ */
