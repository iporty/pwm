#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include <deque>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include "pwm.h"

struct PWMSetting {
  int channel;
  float frac;
  int64_t delay;
  std::chrono::steady_clock::time_point time_to_run;
};

template <typename T, class Derived> class CommandQueueBase {
	public:
    CommandQueueBase<T,Derived>();
    virtual ~CommandQueueBase<T,Derived>();

 		virtual void Start(); // Start the queue processing. 
    void AddCommand(T command);  // add a command to the queue
    
    // Synchronous queue. Making public so derived classes have access
		std::deque<T> _queue;
  
  private:
   
    void ProcessQueue();

    void ProcessCommand(T command) {
			//static_cast<Derived>(this)->ProcessCommandImplementation(T);
		} 

    // Use queue to set PWM in a threadsafe way.
    // dont forget to use lock_guard std::lock_guard _queue_mutex_guard;
    std::mutex _queue_mutex;
    // queue condtion variable
    std::condition_variable _queue_condition;

    // std::thread command_processing_thread_;
    std::thread _command_processing_thread;

    std::mutex _stopping_mutex;
	  bool _stopping;
};

template <typename T> class CommandQueue : public CommandQueueBase<T, CommandQueue<T>> {
  public:
    virtual ~CommandQueue<T>() {};
		void ProcessCommandImplementation(T setting) {
    	printf("Not Fully Specialized\r\n");
  	}
}; 

// Derived classes don't have to worry about threads
// or locking / unlocking queue.
template<> class 
CommandQueue<PWMSetting> : public CommandQueueBase<PWMSetting, CommandQueue<PWMSetting>> {
  public:
    virtual ~CommandQueue() {};

    virtual void StartImplementation() {
 
			_p.init();
			_p.setPWMFreq(50);
			_p.setMin(200);
			_p.setMax(450);

		}
	
		// return if we need to wait
		bool ProcessCommandImplementation(
				std::chrono::microseconds* time_to_wait) {
		  PWMSetting setting(_queue.front());
			std::cout << "Looking at setting frac: " << setting.frac << " queue_size " 
					<< _queue.size() << " delay: " << setting.delay << std::endl;
      std::cout << "Current: " << std::chrono::duration_cast<std::chrono::microseconds>(
                setting.time_to_run - std::chrono::steady_clock::now()).count() << std::endl;
      std::cout << "Here" << std::endl;
			if (setting.delay <= 0 || 
					std::chrono::steady_clock::now() >= setting.time_to_run) { 
				_queue.pop_front();
				_p.setPWM(setting.channel, setting.frac);
    		printf("Setting frac = %f\r\n", setting.frac);
				return false;
			} else {
        printf("waiting\n\r");
				auto start = std::chrono::steady_clock::now();
				auto end = setting.time_to_run;
				*time_to_wait = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
				return true;
			}
  	}

		void AddCommandImplementation(PWMSetting setting) {
      int kStandardDelay = 2000000; // 1 second delay to off
      float kOffFrac = .5;
			std::cout << "Adding Command frac:" << setting.frac << std::endl;
      if (setting.delay <= 0) { 
				setting.time_to_run = std::chrono::steady_clock::now();
      } else {
				setting.time_to_run = std::chrono::steady_clock::now() + 
						std::chrono::microseconds(setting.delay);
			}
      int location = _queue.size() - 1;
      std::cout << "start location" << location << std::endl;
      for (; location >= 0; --location) {
        std::cout << "checking location" << location << std::endl;
        if (_queue.at(location).time_to_run < setting.time_to_run) {
          break;
        }
        if (setting.channel == _queue.at(location).channel) {
            std::cout << "Deleting current: " << std::chrono::duration_cast<std::chrono::microseconds>(
                _queue.at(location).time_to_run - std::chrono::steady_clock::now()).count() << std::endl;
            _queue.erase(_queue.begin() + location);
        }
      }
      std::cout << "Adding at " << location << std::endl;
      std::cout << "Another thing" << std::endl;
			_queue.insert(_queue.begin() + location + 1, setting);
      
      // Push a delayed setting for this channel that will turn it off
      // for safety.
      PWMSetting delayed_off(setting);
      delayed_off.delay += kStandardDelay;
      delayed_off.time_to_run = std::chrono::steady_clock::now() + std::chrono::microseconds(kStandardDelay);
      delayed_off.frac = kOffFrac;
      _queue.push_back(delayed_off);
		}
	private:
		pwm _p;
}; 

template <typename T, class Derived> 
CommandQueueBase<T,Derived>::CommandQueueBase() : _queue(), _stopping(false) {
}

template <typename T, class Derived> 
void CommandQueueBase<T,Derived>::Start() {
  _stopping_mutex.lock();
  if (!_stopping) {
    _command_processing_thread = std::thread(&CommandQueueBase<T,Derived>::ProcessQueue, this);
  } 
  _stopping_mutex.unlock();
  std::cerr << "CQB1";
  static_cast<Derived*>(this)->StartImplementation();
  std::cerr << "CQB2";
}

template <typename T, class Derived>
CommandQueueBase<T,Derived>::~CommandQueueBase() {
  _stopping_mutex.lock();
  _stopping = true;
  _stopping_mutex.unlock();

  // Wake up the thread
  _queue_condition.notify_all();

	_command_processing_thread.join();
}

template <typename T, class Derived>
void CommandQueueBase<T,Derived>::AddCommand(T command) {
	{
		std::lock_guard<std::mutex> lg(_queue_mutex);
  	static_cast<Derived*>(this)->AddCommandImplementation(command);
  }
	_queue_condition.notify_one();
}

// This should only be called on a seperate thread.
template <typename T, class Derived>
void CommandQueueBase<T,Derived>::ProcessQueue() {
  // Is it useful to grab the stopping lock?
  printf("starting the queue processing\r\n");
	std::chrono::microseconds time_to_wait;
  bool timed_wait = false;
	while(!_stopping) {
  	{
			// wait until there is something in the queue;
    	std::unique_lock<std::mutex> lk(_queue_mutex);
      if (timed_wait) {
				timed_wait = false;
      	_queue_condition.wait_for(
						lk, 
						std::chrono::duration_cast<std::chrono::microseconds>(time_to_wait));
			} else {
				_queue_condition.wait(lk);
			}
			while (!_queue.empty() && timed_wait == false) {
				if(static_cast<Derived*>(this)->ProcessCommandImplementation(&time_to_wait)) {
					timed_wait = true;
					std::cout << "Sleeping for " << time_to_wait.count() << "\n";
				} else {
					timed_wait = false;
				}
      }				
		}
	}
}

#endif
