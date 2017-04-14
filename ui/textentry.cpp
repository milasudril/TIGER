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
//@	{"targets":[{"name":"textentry.o","type":"object","pkgconfig_libs":["gtk+-3.0"]}]}

#include "textentry.hpp"
#include "container.hpp"
#include <gtk/gtk.h>

using namespace Tiger;

class TextEntry::Impl
	{
	public:
		Impl(Container& cnt,TextEntry& owner);
		~Impl();

		void callback(Callback cb,void* cb_obj)
			{
			m_cb=cb;
			m_cb_obj=cb_obj;
			}

		const char* content() const noexcept
			{return gtk_entry_get_text(m_handle);}

		void content(const char* text) noexcept
			{return gtk_entry_set_text(m_handle,text);}

		void width(int n) noexcept
			{
			gtk_entry_set_width_chars(m_handle,n);
			gtk_entry_set_max_width_chars(m_handle,n);
			}

		void small(bool status)
			{gtk_entry_set_has_frame(m_handle,!status);}

		void alignment(float x)
			{gtk_entry_set_alignment(m_handle,x);}


	private:
		Callback m_cb;
		void* m_cb_obj;
		TextEntry& r_owner;
		GtkEntry* m_handle;

		static gboolean focus_callback(GtkWidget* widget,GdkEvent* event,gpointer data);
	};

TextEntry::TextEntry(Container& cnt) noexcept
	{
	m_impl=new Impl(cnt,*this);
	}

TextEntry::~TextEntry()
	{
	delete m_impl;
	}

TextEntry& TextEntry::callback(Callback cb,void* cb_obj)
	{
	m_impl->callback(cb,cb_obj);
	return *this;
	}

const char* TextEntry::content() const noexcept
	{return m_impl->content();}

TextEntry& TextEntry::content(const char* text)
	{
	m_impl->content(text);
	return *this;
	}

TextEntry& TextEntry::width(int n) noexcept
	{
	m_impl->width(n);
	return *this;
	}

TextEntry& TextEntry::small(bool status) noexcept
	{
	m_impl->small(status);
	return *this;
	}

TextEntry& TextEntry::alignment(float x) noexcept
	{
	m_impl->alignment(x);
	return *this;
	}



TextEntry::Impl::Impl(Container& cnt,TextEntry& owner):m_cb(nullptr)
	,r_owner(owner)
	{
	printf("Entry %p ctor\n",this);

	auto widget=gtk_entry_new();
	g_signal_connect(widget,"focus-out-event",G_CALLBACK(focus_callback),this);
	m_handle=GTK_ENTRY(widget);
	g_object_ref_sink(widget);
	cnt.add(widget);
	}

TextEntry::Impl::~Impl()
	{
	m_cb=nullptr;
	gtk_widget_destroy(GTK_WIDGET(m_handle));
	printf("Entry %p dtor\n",this);
	}

gboolean TextEntry::Impl::focus_callback(GtkWidget* widget,GdkEvent* event,gpointer data)
	{
	auto state=reinterpret_cast<Impl*>(data);
	if(state->m_cb!=nullptr)
		{(state->m_cb)(state->m_cb_obj,state->r_owner);}
	return FALSE;
	}
