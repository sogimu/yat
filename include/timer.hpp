#pragma once

#include <cstddef>
#include <chrono>
#include <functional>
#include <map>
#include <list>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

namespace yat
{
	class Timer
	{
	private:
		class TimersHandler
		{
		public:
			using TimerMetaInfo = std::tuple<std::chrono::milliseconds, std::chrono::milliseconds, std::function<void()>, std::function<void(std::string)>, bool>;
			static TimersHandler &get()
			{
				static TimersHandler instance;
				return instance;
			}

			TimersHandler(TimersHandler const &) = delete;            // Copy construct
			TimersHandler(TimersHandler &&) = delete;                 // Move construct
			TimersHandler &operator=(TimersHandler const &) = delete; // Copy assign
			TimersHandler &operator=(TimersHandler &&) = delete;      // Move assign
			size_t startServeTo(TimerMetaInfo &&);
			void stopServeTo(size_t taskId);

		private:
			TimersHandler();
			~TimersHandler();
            size_t _newTimerId();

		private:
            std::recursive_mutex m_mutexTimers;
			std::map<size_t, TimerMetaInfo> m_timers;
			std::thread m_threadLoop;
			std::atomic<bool> m_isRunning;
		};

	public:
		Timer(std::function<void(std::string what)> exceptionInfoHandler = [](std::string) {});
		Timer(Timer const &) = delete;              // Copy construct
		Timer &operator=(Timer const &) = delete;   // Copy assign
		~Timer();

		std::chrono::milliseconds interval() const;
		bool isActive() const;
		void setCallback(std::function<void()> callback);
		void setInterval(std::chrono::milliseconds interval);
		void start();
		void start(std::chrono::milliseconds interval);
		void stop();
		void restart();
		void setSingleShot(bool isSingleShot);
		bool isSingleShot() const;

	private:
		size_t m_taskId = 0;
		std::function<void()> m_callback;
		std::function<void(std::string what)> m_exceptionInfoCallback;
		std::chrono::milliseconds m_interval;
        std::atomic<bool> m_isActive;
        std::atomic<bool> m_isSingleShot;
		TimersHandler &m_timersHandler;
	};

}
