///-------------------------------------------------------------------------------------------------
/// File:	include\util\Handle.h.
///
/// Summary:	Declares the handle class.
///-------------------------------------------------------------------------------------------------

#ifndef __HANDLE_H__
#define __HANDLE_H__

#include "API.h"

namespace ECS { namespace util {
	
	namespace Internal
	{
		///-------------------------------------------------------------------------------------------------
		/// Union:	Handle
		///
		/// Summary:	Declares a base handle template class. This template can be specialized in arbitrary 
		/// ways what allows a certain amount of max. possible unique handle versions with a certain amount 
		/// of different versions.
		///
		/// Author:	Tobias Stein
		///
		/// Date:	2/10/2017
		///-------------------------------------------------------------------------------------------------

		template<
			typename handle_version_type,
			typename handle_index_type,
			size_t version_bits,
			size_t index_bits
		>
		union Handle
		{

			using version_type				= handle_version_type;
			using index_type				= handle_index_type;

			static constexpr size_t			NUM_VERSION_BITS	{ version_bits };
			static constexpr size_t			NUM_INDEX_BITS		{ index_bits };


			static constexpr version_type	MIN_VERISON			{ 0 };
			static constexpr version_type	MAX_VERSION			{ (1U << NUM_VERSION_BITS) - 1U };
			static constexpr index_type		MAX_INDICES			{ (1U << NUM_INDEX_BITS)   - 1U };

			static constexpr index_type		INVALID_HANDLE		{ std::numeric_limits<index_type>::max() };

		private:

			index_type value;

		public:

			struct
			{
				index_type					index	: NUM_INDEX_BITS;
				version_type				version : NUM_VERSION_BITS;
			};


			Handle() :
				value(-1)
			{}

			Handle(index_type value) :
				value(value)
			{}

			Handle(index_type index, version_type version) :
				index(index),
				version(version)
			{}

			inline operator index_type() const { return value; }
		};
	} // namespace Internal



	/// Summary:	Defines a 32 bit handle
	/// Max. possible handles							: 1048576
	/// Max. possible versions until loop per handle	: 4096
	using Handle32 = Internal::Handle<u16, u32, 12, 20>;


	/// Summary:	Defines a 64 bit handle
	/// Max. possible handles							: 1099511627776
	/// Max. possible versions until loop per handle	: 16777216
#ifdef ECS_64BIT
	using Handle64 = Internal::Handle<u32, u64, 24, 40>;
#else
	using Handle64 = Handle32;
#endif


	template<class T, class handle_version, size_t grow = 1024>
	class HandleTable
	{
		using Handle = handle_version;

	private:

		using TableEntry = std::pair<typename Handle::version_type, T*>;

		std::vector<TableEntry> m_Table;


		void GrowTable()
		{
			size_t oldSize = this->m_Table.size();

			assert(oldSize < Handle::MAX_INDICES && "Max table capacity reached!");

			size_t newSize = std::min(oldSize + grow, Handle::MAX_INDICES);

			this->m_Table.resize(newSize);

			for (typename Handle::index_type i = oldSize; i < newSize; ++i)
				this->m_Table[i] = TableEntry(Handle::MIN_VERISON, nullptr);
		}

	public:

		HandleTable()
		{
			this->GrowTable();
		}

		~HandleTable()
		{}

		Handle AqcuireHandle(T* rawObject)
		{
			typename Handle::index_type i = 0;
			for (; i < this->m_Table.size(); ++i)
			{
				if (this->m_Table[i].second == nullptr)
				{
					this->m_Table[i].second = rawObject;

					this->m_Table[i].first = ((this->m_Table[i].first + 1) > Handle::MAX_VERSION) ? Handle::MIN_VERISON : this->m_Table[i].first + 1;

					return Handle(i, this->m_Table[i].first);
				}
			}

			// grow table
			this->GrowTable();

			this->m_Table[i].second = rawObject;
			return Handle(i, this->m_Table[i].first++);
		}

		void ReleaseHandle(Handle handle)
		{
			assert((handle.index < this->m_Table.size() && handle.version == this->m_Table[handle.index].first) && "Invalid handle!");
			this->m_Table[handle.index].second = nullptr;
		}

		///-------------------------------------------------------------------------------------------------
		/// Fn:	inline bool HandleTable::IsValidHandle(const Handle& handle) const
		///
		/// Summary:	Check if a 'handle' is expired, that is, its intern version is different from actual
		/// version stored in table.
		///
		/// Author:	Tobias Stein
		///
		/// Date:	1/10/2017
		///
		/// Parameters:
		/// handle - 	The handle.
		///
		/// Returns:	True if valid handle, false if not.
		///-------------------------------------------------------------------------------------------------

		inline bool IsExpired(Handle handle) const
		{
			return this->m_Table[handle.index].first != handle.version;
		}

		///-------------------------------------------------------------------------------------------------
		/// Fn:	Handle HandleTable::operator[](HANDLE_VALUE_TYPE i) const
		///
		/// Summary:	Returns handle from table at position described by i.
		///
		/// Author:	Tobias Stein
		///
		/// Date:	1/10/2017
		///
		/// Parameters:
		/// i - 	Zero-based index of the.
		///
		/// Returns:	The indexed value.
		///-------------------------------------------------------------------------------------------------

		inline Handle operator[](typename Handle::index_type index) const
		{
			assert(index < this->m_Table.size() && "Invalid handle!");
			return Handle(index, this->m_Table[index].first);
		}

		///-------------------------------------------------------------------------------------------------
		/// Fn:	inline T* HandleTable::operator[](Handle handle)
		///
		/// Summary:	Returns the raw object stored for a handle.
		///
		/// Author:	Tobias Stein
		///
		/// Date:	2/10/2017
		///
		/// Parameters:
		/// handle - 	Handle of the handle.
		///
		/// Returns:	The indexed value.
		///-------------------------------------------------------------------------------------------------

		inline T* operator[](Handle handle)
		{
			assert((handle.index < this->m_Table.size() && handle.version == this->m_Table[handle.index].first) && "Invalid handle!");
			return (this->m_Table[handle.index].first == handle.version ? this->m_Table[handle.index].second : nullptr);
		}

	}; // class HandleTable	

}} // namespace ECS::util

#endif // ! __HANDLE_H__