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
//@	{"targets":[{"name":"label.o","type":"object","pkgconfig_libs":["gtk+-3.0"]}]}

#include "label.hpp"
#include "container.hpp"
#include <gtk/gtk.h>

using namespace Tiger;

class Label::Impl
	{
	public:
		Impl(Container& cnt,const char* text);
		~Impl();

		const char* content() const noexcept
			{return gtk_label_get_text(m_handle);}

		void content(const char* text)
			{gtk_label_set_text(m_handle,text);}

	private:
		GtkLabel* m_handle;
	};

Label::Label(Container& cnt,const char* text)
	{
	m_impl.reset(new Impl(cnt,text));
	}

Label::~Label()
	{}

const char* Label::content() const noexcept
	{return m_impl->content();}

Label& Label::content(const char* x)
	{
	m_impl->content(x);
	return *this;
	}

Label::Impl::Impl(Container& cnt,const char* text)
	{
	printf("Label %p ctor\n",this);

	auto widget=gtk_label_new(text);
	
	m_handle=GTK_LABEL(widget);
	g_object_ref_sink(m_handle);
	cnt.add(widget);
	}

Label::Impl::~Impl()
	{
	gtk_widget_destroy(GTK_WIDGET(m_handle));
	printf("Label %p dtor\n",this);
	}
