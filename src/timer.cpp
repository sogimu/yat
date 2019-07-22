#include "timer.hpp"

#include <iostream>
#include <cassert>

using namespace yat;

Timer::Timer(std::function<void(std::string what)> exceptionInfoHandler)
	: m_exceptionInfoCallback(exceptionInfoHandler)
	, m_interval(1)
    , m_isActive(false)
    , m_isSingleShot(false)
	, m_timersHandler(TimersHandler::get())
{
}

Timer::~Timer()
{
	stop();
}

std::chrono::milliseconds Timer::interval() const
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;
    return m_interval;
}

bool Timer::isActive() const
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;
    return m_isActive;
}

bool Timer::isSingleShot() const
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;
    return m_isSingleShot;
}

void Timer::setCallback(std::function<void()> callback)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    m_callback = callback;
	if (m_isActive)
	{
		restart();
	}
}

void Timer::setInterval(std::chrono::milliseconds interval)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    assert(interval > std::chrono::milliseconds(0));
	m_interval = interval;
	if (m_isActive)
	{
		restart();
	}
}

void Timer::setSingleShot(bool isSingleShot)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    m_isSingleShot = isSingleShot;
	if (m_isActive)
	{
		restart();
	}
}

void Timer::start()
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

	if (m_isActive)
	{
		stop();
	}
	if (m_interval.count() > 0)
	{
		auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
		m_taskId = m_timersHandler.startServeTo({now, m_interval, m_callback, m_exceptionInfoCallback, m_isSingleShot});
	}
	m_isActive = true;
}

void Timer::stop()
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    if (m_taskId != 0)
	{
		m_timersHandler.stopServeTo(m_taskId);
	}
	m_isActive = false;
}

void Timer::restart()
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    stop();
	start();
}

void Timer::start(std::chrono::milliseconds interval)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    setInterval(interval);
	start();
}

Timer::TimersHandler::TimersHandler()
{
	m_threadLoop = std::thread([this]()
	{
		m_isRunning = true;
		while(m_isRunning)
		{
		    try
		    {
//                std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;
                std::list<std::function<void()>> callbacks;

//                std::lock_guard<std::recursive_mutex> lock(m_mutexTimers);
                m_mutexTimers.lock();
                {
                    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
//                m_mutexTimers.lock();
//                if (m_timers.size() > 0)
//                {
//                    std::cout << "Have timers! " << m_isRunning << ", " << std::this_thread::get_id() << ", " << m_timers.size() << std::endl;
//                }

                    for (auto it = m_timers.begin(); it != m_timers.end();)
                    {
                        std::chrono::milliseconds addingTime = std::get<0>(it->second);
                        std::chrono::milliseconds interval = std::get<1>(it->second);
                        std::function<void()> callback = std::get<2>(it->second);
                        std::function<void(std::string)> exceptionInfoCallback = std::get<3>(it->second);
                        bool isSingleShot = std::get<4>(it->second);

//                    m_mutexTimers.unlock();

//                    std::cout << "Check timer! " << m_isRunning << ", " << std::this_thread::get_id() << std::endl;

                        bool isEraseNeeded = false;
                        std::chrono::milliseconds passedAfterAdding = now - addingTime;
                        bool timeIsCame =
                         (now >= addingTime + interval) and (passedAfterAdding.count() % interval.count() == 0);
                        if (timeIsCame)
                        {
                            try
                            {
//                                std::cout << "Going exec timers callback, " << std::this_thread::get_id() << std::endl;
//                            callback();
                                callbacks.push_back(callback);
                            }
                            catch (std::exception &e)
                            {
                                std::cout << __PRETTY_FUNCTION__ << e.what() << std::endl;
//                            exceptionInfoCallback(e.what());
                            }

                            isEraseNeeded = isSingleShot;
                        }

                        if (isEraseNeeded)
                        {
//                        std::lock_guard<std::recursive_mutex> lock(m_mutexTimers);
                            if (m_timers.find(it->first) != m_timers.end())
                            {
                                it = m_timers.erase(it);
                            }
                        }
                        else
                        {
//                        std::lock_guard<std::recursive_mutex> lock(m_mutexTimers);
                            it++;
                        }
//                    m_mutexTimers.lock();

                    }
                }

                m_mutexTimers.unlock();

                for (auto callback : callbacks)
                {
                    callback();
                }

//                std::cout << m_isRunning << ", " << std::this_thread::get_id() << ", " << m_timers.size() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
		    }
		    catch(std::exception & e)
            {
		        std::cout << __PRETTY_FUNCTION__ << e.what() << std::endl;
            }
		}

		std::cout << __PRETTY_FUNCTION__ << "!!!THREAR EXIT!!!" << std::endl;
    });
//	m_threadLoop.detach();
}

Timer::TimersHandler::~TimersHandler()
{
//    std::cout << __PRETTY_FUNCTION__ << std::endl;

    m_isRunning = false;
	if (m_threadLoop.joinable())
	{
		m_threadLoop.join();
	}
}

size_t Timer::TimersHandler::_newTimerId()
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    size_t taskId = 0;
//    for (auto & pair : m_timers)
    try
    {
        for (auto it = m_timers.begin(); it!=m_timers.end();)
        {
            if (it->first > taskId)
            {
                taskId = it->first;
            }
            it++;
        }
        taskId++;
    }
    catch(std::exception & e)
    {
        std::cout << __PRETTY_FUNCTION__ << e.what() << std::endl;
    }

    return taskId;
}

size_t Timer::TimersHandler::startServeTo(TimerMetaInfo &&timerMetaInfo)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;

    size_t taskId = 0;

    try
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutexTimers);
        taskId = _newTimerId();
        m_timers.emplace(taskId, timerMetaInfo);
    }
    catch(std::exception & e)
    {
        std::cout << __PRETTY_FUNCTION__ << e.what() << std::endl;
    }
    return taskId;
}

void Timer::TimersHandler::stopServeTo(size_t taskId)
{
//    std::cout << __PRETTY_FUNCTION__ << "id=" << std::this_thread::get_id() << std::endl;
    try
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutexTimers);
        if (m_timers.find(taskId) != m_timers.end())
        {
            m_timers.erase(m_timers.find(taskId));
        }
    }
    catch(std::exception & e)
    {
        std::cout << __PRETTY_FUNCTION__ << e.what() << std::endl;
    }

//    std::cout << __PRETTY_FUNCTION__ << m_timers.size() << std::endl;
}
