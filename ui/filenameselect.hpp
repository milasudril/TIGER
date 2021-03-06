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
//@	 "targets":{"name":"filenamepicker.hpp","type":"include"}
//@	,"dependencies_extra":[{"ref":"filenameselect.o","rel":"implementation"}]
//@	}

#ifndef TIGER_FILENAMEPICKER_HPP
#define TIGER_FILENAMEPICKER_HPP

#include <utility>
#include <string>

namespace Tiger
	{
	class Container;

	enum class FileselectMode:int{OPEN,SAVE};

	typedef bool (*filtercb)(void* cb_obj,const char* filename_check);
	bool filenameSelect(const Container& cnt,std::string& filename_in,FileselectMode mode
		,filtercb cb,void* cb_obj,const char* filter_name);

	inline bool filenameSelect(const Container& cnt,std::string& filename_in,FileselectMode mode)
		{
		auto cb=[](void* cb_obj,const char* filename_check)
			{return true;};
		return filenameSelect(cnt,filename_in,mode,cb,nullptr,"");
		}

	template<class FilterCallback>
	inline bool filenameSelect(const Container& cnt,std::string& filename_in,FileselectMode mode
		,FilterCallback&& filter,const char* filter_name)
		{
		auto cb=[](void* cb_obj,const char* filename_check)
			{
			auto filter=reinterpret_cast<FilterCallback*>(cb_obj);
			return (*filter)(filename_check);
			};
		return filenameSelect(cnt,filename_in,mode,cb,&filter,filter_name);
		}


	}

#endif