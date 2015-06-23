#pragma once

#include "ComPtr.h"

namespace Utilities
{
	namespace Com
	{
		template <typename ...Interfaces>
		struct ComClass : public Interfaces...
		{
		private:
			uint32_t m_ReferenceCount;

		protected:
			ComClass() :
				m_ReferenceCount(1)
			{
			}

		private:
			static void* Allocate()
			{
				return new uint8_t[sizeof(DerivedType)];
			}

			virtual void DeallocateThis()
			{
				delete[] reinterpret_cast<uint8_t*>(this);
			}

			template <int dummy = 0>
			inline HRESULT QueryInterfaceImpl(REFIID riid, void** ppvObject)
			{
				UNREFERENCED(riid);

				*ppvObject = nullptr;
				return E_NOINTERFACE;
			}

			template <typename FirstInterface, typename ...RestInterfaces>
			inline HRESULT QueryInterfaceImpl(REFIID riid, void** ppvObject)
			{
				if (__uuidof(FirstInterface) == riid)
				{
					*ppvObject = static_cast<FirstInterface*>(this);
					AddRef();
					return S_OK;
				}

				return QueryInterfaceImpl<RestInterfaces...>(riid, ppvObject);
			}

		public:
			virtual ~ComClass()
			{
			}

			virtual ULONG STDMETHODCALLTYPE AddRef() override
			{
				auto newReferenceCount = InterlockedIncrement(&m_ReferenceCount);
				return newReferenceCount;
			}

			virtual ULONG STDMETHODCALLTYPE Release() override
			{
				auto newReferenceCount = InterlockedDecrement(&m_ReferenceCount);

				if (newReferenceCount == 0)
				{
					this->~ComClass();
					this->DeallocateThis();
				}

				return newReferenceCount;
			}

			virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
			{
				if (riid == __uuidof(IUnknown))
				{
					*ppvObject = this;
					AddRef();
					return S_OK;
				}

				return QueryInterfaceImpl<Interfaces...>(riid, ppvObject);
			}

			template <typename Type, typename ...Args>
			friend inline ComPtr<Type> Make(Args&&... args)
			{
				auto memory = Type::Allocate();
				return new (memory)DerivedType(std::forward<Args>(args)...);
			}
		};
	}
}