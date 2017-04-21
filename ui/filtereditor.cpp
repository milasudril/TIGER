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
//@	{"targets":[{"name":"filtereditor.o","type":"object"}]}

#include "filtereditor.hpp"
#include "filenameselect.hpp"
#include "messageshow.hpp"
#include "../engine/srcstdio.hpp"
#include "../engine/sinkstdio.hpp"
#include "../engine/blob.hpp"
#include "../engine/mimeidentifier.hpp"

using namespace Tiger;

TIGER_BLOB(char,example,"engine/example.cpp");

FilterEditorBase::FilterEditorBase(Container& cnt):m_box(cnt,1)
	,m_toolbar(m_box,0,0)
	,m_vsplit(m_box.insertMode({0,Box::FILL|Box::EXPAND}),0)
		,m_src_view(m_vsplit.insertMode({Paned::RESIZE|Paned::SHRINK_ALLOWED}),0)
		,m_output(m_vsplit,1),m_dirty(0)
	{
	m_toolbar.append("New").append("Open").append("Save").append("Save As")
		.append("Compile").append("Load").callback(*this);
	m_src_view.highlight("foo.cpp").lineNumbers(1);
	m_output.content("Click \"Compile\" or \"Load\" to compile the filter")
		.readonly(1);
	create();
	m_src_view.callback(*this);
	}

void FilterEditorBase::create()
	{
	if(askSave())
		{
		m_src_view.content(example_begin);
		m_dirty=0;
		stateChangeNotify();
		}
	}

void FilterEditorBase::open()
	{
	if(askSave())
		{
		MimeIdentifier mime;
		if(filenameSelect(m_box,m_file_current,FileselectMode::OPEN
			,[&mime](const char* filename)
				{
				auto m=mime.identify(filename);
				return begins_with(m,"text/x-c; charset=utf-8") 
					|| begins_with(m,"text/plain; charset=utf-8");
				},"Filter source files"))
			{
			m_src_view.content(SrcStdio{m_file_current.c_str()});
			m_dirty=0;
			stateChangeNotify();
			}
		}
	}

bool FilterEditorBase::saveAs()
	{
	std::string filename_new;
	if(filenameSelect(m_box,filename_new,FileselectMode::SAVE))
		{
		save(filename_new);
		m_file_current=filename_new;
		stateChangeNotify();
		return 1;
		}
	return 0;
	}

bool FilterEditorBase::save()
	{
	if(m_file_current.size()==0)
		{return saveAs();}
	save(m_file_current);
	stateChangeNotify();
	return 1;
	}

void FilterEditorBase::save(std::string filename)
	{
	SinkStdio dest(filename.c_str());
	dest.write(m_src_view.content(),m_src_view.contentSize());
	m_dirty=0;
	}

void FilterEditorBase::operator()(ButtonList<FilterEditorBase>& src,Button& btn)
	{
	try
		{
		switch(btn.id())
			{
			case 0:
				create();
				break;
			case 1:
				open();
				break;
			case 2:
				save();	
				break;
			case 3:
				saveAs();
				break;
			default:
				break;
			}
		}
	catch(const Tiger::Error& msg)
		{messageShow(m_box,msg.message(),"Tiger",MESSAGE_TYPE_ERROR|MESSAGE_BUTTONS_OK);}
	btn.state(0);
	}

void FilterEditorBase::operator()(SourceView& srcv)
	{
	m_dirty=1;
	stateChangeNotify();
	}

bool FilterEditorBase::askSave()
	{
	if(m_dirty)
		{
		std::string message("Do you want to save changes to `");
		message+=m_file_current;
		message+="`?";
		auto ans=messageShow(m_box,message.c_str(),"Tiger"
			,MESSAGE_TYPE_WARNING|MESSAGE_BUTTONS_YESNOCANCEL);
		switch(ans)
			{
			case MessageAnswer::CANCEL:
				return 0;
			case MessageAnswer::NO:
				return 1;
			case MessageAnswer::YES:
				return save();
			default:
				return 0;
			}
		}
	return 1;
	}
