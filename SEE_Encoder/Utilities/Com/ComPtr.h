#pragma once

namespace Utilities
{
	namespace Com
	{
		template <typename T>
		class ComPtr;

		template <typename T>
		class ComPtrRef
		{
		private:
			ComPtr<T>& m_ComPtr;

			ComPtrRef(ComPtr<T>& comPtr) :
				m_ComPtr(comPtr)
			{
			}

			friend class ComPtr<T>;

		public:
			inline operator T**()
			{
				m_ComPtr.Release();
				return m_ComPtr.GetAddressOf();
			}

			inline operator void**()
			{
				m_ComPtr.Release();
				return reinterpret_cast<void**>(m_ComPtr.GetAddressOf());
			}

			ComPtrRef(const ComPtrRef&) = delete;
			ComPtrRef& operator=(const ComPtrRef&) = delete;
		};

		template <typename T>
		class ComPtr
		{
		private:
			T* m_Ptr;

			inline void Release()
			{
				if (m_Ptr)
					m_Ptr->Release();

				m_Ptr = nullptr;
			}

			inline void AddRef()
			{
				if (m_Ptr)
					m_Ptr->AddRef();
			}

			friend class ComPtrRef<T>;

		public:
			inline ComPtr() :
				m_Ptr(nullptr)
			{
			}

			inline ComPtr(decltype(nullptr)) :
				m_Ptr(nullptr)
			{
				AddRef();
			}

			template <typename U>
			inline ComPtr(U* ptr) :
				m_Ptr(ptr)
			{
				AddRef();
			}

			template <typename U>
			inline ComPtr(const ComPtr<U>& other) :
				m_Ptr(other.m_Ptr)
			{
				AddRef();
			}

			template <typename U>
			inline ComPtr(ComPtr<U>&& other) :
				m_Ptr(other.m_Ptr)
			{
				other.m_Ptr = nullptr;
			}

			inline ~ComPtr()
			{
				Release();
			}

			inline ComPtr<T>& operator=(decltype(nullptr))
			{
				if (m_Ptr != nullptr)
					Release();

				return *this;
			}


			template <typename U>
			inline ComPtr<T>& operator=(U* ptr)
			{
				if (m_Ptr != ptr)
				{
					Release();
					m_Ptr = ptr;
					AddRef();
				}

				return *this;
			}

			template <typename U>
			inline ComPtr<T>& operator=(const ComPtr<U>& other)
			{
				if (m_Ptr != other.m_Ptr)
				{
					Release();
					m_Ptr = other.m_Ptr;
					AddRef();
				}

				return *this;
			}

			template <typename U>
			inline ComPtr<T>& operator=(ComPtr<U>&& other)
			{
				Release();
				m_Ptr = other.m_Ptr;
				return *this;
			}

			inline T* operator->()
			{
				return m_Ptr;
			}

			inline operator T*()
			{
				return m_Ptr;
			}

			inline operator const T*() const
			{
				return m_Ptr;
			}

			inline T** GetAddressOf()
			{
				return &m_Ptr;
			}

			inline T* const* GetAddressOf() const
			{
				return &m_Ptr;
			}

			inline T*& GetReferenceToPtr()
			{
				return m_Ptr;
			}

			inline ComPtrRef<T> operator&()
			{
				return ComPtrRef<T>(*this);
			}
			
			template<typename U>
			inline HRESULT As(typename ComPtrRef<U> p)
			{
				return m_Ptr->QueryInterface(__uuidof(U), p);
			}
		};
	}
}