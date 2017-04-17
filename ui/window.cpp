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

class Window::Impl:private Window
	{
	public:
		Impl(const char* ti,int id);
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

		void _show()
			{gtk_widget_show_all(GTK_WIDGET(m_handle));}

		void _sensitive(bool val)
			{gtk_widget_set_sensitive(GTK_WIDGET(m_handle),val);}

		void callback(Callback cb,void* cb_obj)
			{
			m_cb=cb;
			m_cb_obj=cb_obj;
			}
			
		int id() const noexcept
			{return m_id;}

	private:
		static gboolean delete_callback(GtkWidget* widget,GdkEvent* event,gpointer user_data);
		int m_id;
		Callback m_cb;
		void* m_cb_obj;
		GtkWindow* m_handle;
		std::string m_title;
	};

Window::Window(const char* title,int id)
	{m_impl=new Impl(title,id);}

Window::~Window()
	{delete m_impl;}

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

Window& Window::show()
	{
	m_impl->_show();
	return *this;
	}

Window& Window::sensitive(bool val)
	{
	m_impl->_sensitive(val);
	return *this;
	}

int Window::id() const noexcept
	{return m_impl->id();}

Window& Window::callback(Callback cb,void* cb_obj)
	{
	m_impl->callback(cb,cb_obj);
	return *this;
	}



Window::Impl::Impl(const char* ti,int id):Window(*this),m_cb(nullptr)
	{
	printf("Window %p ctor\n",this);
	auto widget=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(widget,"delete-event",G_CALLBACK(delete_callback),this);
	m_handle=GTK_WINDOW(widget);
	m_id=id;
	title(ti);
	}

Window::Impl::~Impl()
	{
	m_impl=nullptr;
	printf("Window %p dtor\n",this);
	m_cb=nullptr;
	gtk_widget_destroy(GTK_WIDGET(m_handle));
	}

gboolean Window::Impl::delete_callback(GtkWidget* widget,GdkEvent* event,gpointer user_data)
	{
	auto self=reinterpret_cast<Impl*>(user_data);
	if(self->m_cb!=nullptr)
		{self->m_cb(self->m_cb_obj,*self);}
	return TRUE;
	}
