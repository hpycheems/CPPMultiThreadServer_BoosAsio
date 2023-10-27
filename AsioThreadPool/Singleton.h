#pragma once
#include <iostream>
#include <mutex>
#include <memory>

template<typename T>
class Singleton {
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton<T>& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance() {
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]{_instance = std::shared_ptr<T>(new T); });
		return _instance;
	}

	void PrindAddress() {
		std::cout << _instance.get() << "\n";
	}

	~Singleton() {
		std::cout << "Singleton Îö¹¹\n";
	}
};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;