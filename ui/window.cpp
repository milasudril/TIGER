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
//@  "targets":[{"name":"window.o","type":"object","pkgconfig_libs":["gtk+-3.0"]}]
//@	}

#include "window.hpp"
#include <gtk/gtk.h>
#include <string>

using namespace Tiger;

class Window::Impl
	{
	public:
		Impl(const char* ti,bool mainwin);
		~Impl();

		const char* title() const noexcept
			{return m_title.c_str();}

		void title(const char* title_new)
			{
			gtk_window_set_title(m_handle,title_new);
			m_title=title_new;
			}

		void add(GtkWidget* handle)
			{gtk_container_add(GTK_CONTAINER(m_handle),handle);}

		void show()
			{gtk_widget_show_all(GTK_WIDGET(m_handle));}

	private:
		static void destroy_callback (GtkWidget* object,gpointer user_data);
		bool m_mainwin;
		GtkWindow* m_handle;
		std::string m_title;
	};

Window::Window(const char* title,bool mainwin)
	{
	m_impl=new Window::Impl(title,mainwin);
	}

Window::~Window()
	{
	delete m_impl;
	}

const char* Window::title() const noexcept
	{return m_impl->title();}

Window& Window::title(const char* title_new)
	{
	m_impl->title(title_new);
	return *this;
	}


Window& Window::add(void* handle)
	{
	m_impl->add(GTK_WIDGET(handle));
	return *this;
	}

void Window::show()
	{
	m_impl->show();
	}

Window::Impl::Impl(const char* ti,bool mainwin):m_mainwin(mainwin)
	{
	printf("Window %p ctor\n",this);
	auto widget=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(widget,"destroy",G_CALLBACK(destroy_callback),this);
	m_handle=GTK_WINDOW(widget);
	title(ti);
	}

Window::Impl::~Impl()
	{
	if(m_mainwin)
		{gtk_main();}
	printf("Window %p dtor\n",this);
	}

void Window::Impl::destroy_callback (GtkWidget* object,gpointer user_data)
	{
	auto self=reinterpret_cast<Impl*>(user_data);
	if(self->m_mainwin)
		{gtk_main_quit();}
	}
