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
//@	{"targets":[{"name":"client.hpp","type":"include"}]}

#ifndef TIGER_CLIENT_HPP
#define TIGER_CLIENT_HPP

#include "host.hpp"

namespace Tiger
	{
	template<int N,int M>
	class ProcessDataClientImpl:protected ProcessData
		{
		public:
			using ProcessData::width;
			using ProcessData::height;

			template<int n>
			float& value_next(int x,int y) const noexcept
				{
				static_assert(n>=0,"Channel not found");
				static_assert(n<N,"Channel count out of bounds");
				return m_next[N*(y*m_height + x) + n];
				}

			template<int n>
			float value_current(int x,int y) const noexcept
				{
				static_assert(n>=0,"Channel not found");
				static_assert(n<N,"Channel count out of bounds");
				return m_current[N*(y*m_height + x) + n];
				}

			template<int n>
			float source(int x,int y) const noexcept
				{
				static_assert(n>=0,"Channel not found");
				static_assert(n<N,"Channel count out of bounds");
				return m_src[N*(y*m_height + x) + n];
				}

			template<int n>
			float param() const noexcept
				{
				static_assert(n>=0,"Parameter not found");
				static_assert(n<M,"Parameter count out of bounds");
				return m_params[n];
				}
			};
	}

#define TIGER_PARAMETERS(...) \
	static constexpr const char* __param_names[]\
	={ __VA_ARGS__,nullptr};\
	const char* const* parameters() noexcept {return __param_names;}\
	static constexpr int P(const char* x) {return map(__param_names,x);}

#define TIGER_CHANNELS(...) static constexpr const char* __channel_names[]\
	={ __VA_ARGS__,nullptr};\
	const char* const* channels() noexcept {return __channel_names;}\
	static constexpr int C(const char* x){return map(__channel_names,x);}\

#define TIGER_ENTRY(entrypoint) \
	namespace Tiger \
		{ \
		typedef ProcessDataClientImpl<count(__channel_names),count(__param_names)> ProcessDataClient; \
		} \
		static void entrypoint(const Tiger::ProcessDataClient& data); \
	namespace Tiger\
		{ \
		void __process(const Tiger::ProcessData& data) \
			{ \
			entrypoint(reinterpret_cast<const ProcessDataClient&>(data)); \
			} \
		} \

static constexpr bool equals(const char* a,const char* b)
	{
	while(1)
		{
		if(*a=='\0')
			{break;}
		if(*b=='\0')
			{break;}
		if(*a!=*b)
			{return 0;}
		++a;
		++b;
		}
	return *a=='\0' && *b=='\0';
	}

static constexpr int map(const char* const* x,const char* name)
	{
	auto ptr=x;
	while((*ptr)!=nullptr)
		{
		if(equals(*ptr,name))
			{return ptr-x;}
		++ptr;
		}
	return -1;
	}

static constexpr int count(const char* const* x)
	{
	auto ptr=x;
	while((*ptr)!=nullptr)
		{++ptr;}
	return ptr-x;
	}

#endif