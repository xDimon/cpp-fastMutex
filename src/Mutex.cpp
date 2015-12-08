/*
 * Mutex.cpp
 *
 *  Created on: 22 июля 2014 г.
 *      Author: Dmitriy Khaustov aka Dиmon (khaustov.dm@gmail.com)
 */

#include "Mutex.hpp"

#include <signal.h>

#include "MutexInfo.hpp"
#include "MutexWatcher.hpp"

#define COUNTDOWN (1<<14)

MutexWatcher Mutex::_watcher = {
	false,
	nullptr,
	nullptr
};

bool Mutex::setWatcher(
	MutexInfo *(*push)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n, bool captured),
	bool (*pop)(const Mutex *mutex, const char *place, uint16_t line, uint16_t n)
)
{
	if (!__sync_bool_compare_and_swap(&_watcher.set, false, true))
	{
		return false;
	}
	_watcher.set = true;
	_watcher.push = push;
	_watcher.pop = pop;
	return true;
}

Mutex::Mutex(const char *name)
: _name(name), _mutex(false), _owner(0), _count(0)
{
}

Mutex::~Mutex()
{
	if (!__sync_bool_compare_and_swap(&_mutex, false, true))
	{
//		Log::errorf(0, "Destruction of locked mutex");
		raise(SIGUSR2);
		throw "Destruction of locked mutex";
	}
}

int Mutex::_lock(const char *place, int line)
{
	pthread_t current = pthread_self();
	int countdown = COUNTDOWN;
	int n;
	MutexInfo *info = nullptr;

	for (;;)
	{
		if (__sync_bool_compare_and_swap(&_mutex, false, true))
		{
			// Захват
			if (!_owner)
			{
				n = _count = 1;
				_owner = current;

				if (_watcher.set && place)
				{
					if (!info)
					{
						_watcher.push(this, place, line, n, true);
					}
					else
					{
						info->capture(n);
					}
				}

				__sync_bool_compare_and_swap(&_mutex, true, false);
				return n;
			}

			// Рекурсивный захват
			if (_owner == current)
			{
				n = ++_count;

				if (_watcher.set && place)
				{
					if (!info)
					{
						_watcher.push(this, place, line, n, true);
					}
					else
					{
						info->capture(n);
					}
				}

				__sync_bool_compare_and_swap(&_mutex, true, false);
				return n;
			}

			__sync_bool_compare_and_swap(&_mutex, true, false);
		}

		// Захватить "с ходу" не удалось
		if (_watcher.set && place)
		{
			if (!info)
			{
				info = _watcher.push(this, place, line, 0, false);
			}
		}

		// Цикл активного ожидания
		if (!--countdown)
		{
			countdown = COUNTDOWN;
			pthread_yield();
		}
	}
	return n;
}

int Mutex::_trylock(const char *place, int line)
{
	pthread_t current = pthread_self();
	int countdown = COUNTDOWN;
	int n;

	while(!__sync_bool_compare_and_swap(&_mutex, false, true))
	{
		// Цикл активного ожидания
		if (!--countdown)
		{
			return 0;
		}
	}

	// Захват
	if (!_owner)
	{
		n = _count = 1;
		_owner = current;

		if (_watcher.set && place)
		{
			_watcher.push(this, place, line, n, true);
		}

		__sync_bool_compare_and_swap(&_mutex, true, false);
		return n;
	}

	// Рекурсивный захват
	if (_owner == current)
	{
		n = ++_count;

		if (_watcher.set && place)
		{
			_watcher.push(this, place, line, n, true);
		}

		__sync_bool_compare_and_swap(&_mutex, true, false);
		return n;
	}

	__sync_bool_compare_and_swap(&_mutex, true, false);
	return 0;
}

int Mutex::_unlock(const char *place, int line)
{
	pthread_t current = pthread_self();
	int countdown = COUNTDOWN;

	while(!__sync_bool_compare_and_swap(&_mutex, false, true))
	{
		// Цикл активного ожидания
		if (!--countdown)
		{
			countdown = COUNTDOWN;
			pthread_yield();
		}
	}

	// Ошибка - только владелец может разлочить
	if (_owner != current)
	{
		__sync_bool_compare_and_swap(&_mutex, true, false);
		throw;
		return -1;
	}

	// Полностью разлочен - забываем владельца
	if (!--_count)
	{
		_owner = 0;
	}

	int n = _count;

	if (_watcher.set && place)
	{
		_watcher.pop(this, place, line, n);
	}

	__sync_bool_compare_and_swap(&_mutex, true, false);

	return n;
}

void Mutex::lock(const char *place, int line)
{
	_lock(place, line);
}

bool Mutex::trylock(const char *place, int line)
{
	return _trylock(place, line) > 0;
}

void Mutex::unlock(const char *place, int line)
{
	_unlock(place, line);
}

void Mutex::lock()
{
	_lock(nullptr, 0);
}

bool Mutex::trylock()
{
	return _trylock(nullptr, 0) > 0;
}

void Mutex::unlock()
{
	_unlock(nullptr, 0);
}
