/*
Tiger
Copyright (C) 2017  Torbjörn Rathsman

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
//@	{
//@	 "targets":[{"name":"mapview.hpp","type":"include"}]
//@	,"dependencies_extra":[{"ref":"mapview.o","rel":"implementation"}]
//@	}

#ifndef TIGER_MAPVIEW_HPP
#define TIGER_MAPVIEW_HPP

#include <type_traits>
#include <string>
#include <memory>

namespace Tiger
	{
	class Container;

	class MapViewBase
		{
		public:
			typedef std::string (*StringCallback)(const void* user_data,const void* value);
			typedef void (*ValueSetCallback)(void* src_obj,void* user_data
				,void* ptr,const char* value_new);

			struct DataDescriptorImpl
				{
				StringCallback string_from_key;
				StringCallback string_from_value;
				const void* cb_user_data;
				const char* key;
				const char* value;
				bool value_const;
				};

			MapViewBase(const MapViewBase&)=delete;
			MapViewBase& operator=(const MapViewBase&)=delete;

			explicit MapViewBase(Container& cnt,int id,const DataDescriptorImpl& descriptor);
			
			~MapViewBase();

			MapViewBase& keyAlignment(float val);

			MapViewBase& valueAlignment(float val);

			MapViewBase& recordAppend(const void* key,const void* value);

			MapViewBase& clear();

			MapViewBase& callback(ValueSetCallback cb,void* cb_obj);
			
			int id() const noexcept;

		private:
			class Impl;
			std::unique_ptr<Impl> m_impl;
		};

	template<class DataDescriptor>
	class MapView:protected MapViewBase
		{
		public:
			typedef typename DataDescriptor::key_type key_type;
			typedef typename DataDescriptor::mapped_type mapped_type;

			explicit MapView(Container& cnt,DataDescriptor&& desc)=delete;
			explicit MapView(Container& cnt,int id,const DataDescriptor& desc):
				MapViewBase(cnt,id
					,{
					 [](const void* user_data,const void* key)
						{
						auto self=reinterpret_cast<const DataDescriptor*>(user_data);
						auto x=reinterpret_cast<const key_type*>(key);
						return self->keyString(*x);
						}
					,[](const void* user_data,const void* value)
						{
						auto self=reinterpret_cast<const DataDescriptor*>(user_data);
						auto x=reinterpret_cast<const mapped_type*>(value);
						return self->valueString(*x);
						}
					,&desc
					,desc.key()
					,desc.value()
					,std::is_const<mapped_type>::value
					}){}

			MapView& recordAppend(const key_type& key,mapped_type& value)
				{
				MapViewBase::recordAppend(&key,&value);
				return *this;
				}

			template<class Callback>
			std::enable_if_t<!std::is_const<mapped_type>::value,MapView&>
			callback(Callback& cb) noexcept
				{
				auto func=[](void* src_obj,void* user_data
					,void* ptr,const char* value_new)
					{
					auto self=reinterpret_cast<MapView*>(src_obj);
					auto cb_obj=reinterpret_cast<Callback*>(user_data);
					auto x=reinterpret_cast<mapped_type*>(ptr);
					(*cb_obj)(*self,x,value_new);
					};
				MapViewBase::callback(func,&cb);
				return *this;
				}

			MapView& valueAlignment(float x) noexcept
				{
				MapViewBase::valueAlignment(x);
				return *this;
				}
			
			MapView& keyAlignment(float x) noexcept
				{
				MapViewBase::keyAlignment(x);
				return *this;
				}

			MapView& clear() noexcept
				{
				MapViewBase::clear();
				return *this;
				}
				
			using MapViewBase::id;
		};
	}

#endif
