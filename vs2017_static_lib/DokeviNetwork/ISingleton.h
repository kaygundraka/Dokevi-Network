#pragma once

namespace DokeviNet
{
    template <class T>
    class ISingleton
	{
    protected:
		ISingleton() : _released(false) {}

		virtual ~ISingleton()
		{
			if (!_released)
				Finalize();
		}

		bool _released;

	private:
		static T* _inst;

		void RealRelease()
        {
            Finalize();
            _released = true;
        }

    public:
		ISingleton(const ISingleton& right) = delete;

		virtual void Initialize() {}
		virtual void Finalize() {}

        static T* GetInstance()
        {
            if (_inst == nullptr)
                _inst = new T();
            
            return _inst;
        }
    };

    template <class T>
    T* ISingleton<T>::_inst = nullptr;
}