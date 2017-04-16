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
//@	{"targets":[{"name":"button.o","type":"object","pkgconfig_libs":["gtk+-3.0"]}]}

#include "button.hpp"
#include "container.hpp"
#include <gtk/gtk.h>

using namespace Tiger;

class Button::Impl
	{
	public:
		Impl(Container& cnt,int id,const char* label,Button& owner);
		~Impl();	

		void callback(Callback cb,void* cb_obj)
			{
			m_cb=cb;
			m_cb_obj=cb_obj;
			}

		const char* label() const noexcept
			{return gtk_button_get_label(m_handle);}

		void label(const char* text) noexcept
			{return gtk_button_set_label(m_handle,text);}

		int id() const noexcept
			{return m_id;}

	private:
		int m_id;
		Callback m_cb;
		void* m_cb_obj;
		Button& r_owner;
		GtkButton* m_handle;

		static void clicked_callback(GtkWidget* widget,gpointer data);
	};

Button::Button(Container& cnt,int id,const char* label)
	{m_impl.reset(new Impl(cnt,id,label,*this));}

Button::~Button()
	{}

Button& Button::callback(Callback cb,void* cb_obj)
	{
	m_impl->callback(cb,cb_obj);
	return *this;
	}

const char* Button::label() const noexcept
	{return m_impl->label();}

Button& Button::label(const char* text)
	{
	m_impl->label(text);
	return *this;
	}
	
int Button::id() const noexcept
	{return m_impl->id();}


Button::Impl::Impl(Container& cnt,int id,const char* lab,Button& owner):m_id(id),m_cb(nullptr)
	,r_owner(owner)
	{
	printf("Button %p ctor\n",this);

	auto widget=gtk_button_new();
	g_signal_connect(widget,"clicked",G_CALLBACK(clicked_callback),this);
	m_handle=GTK_BUTTON(widget);
	g_object_ref_sink(widget);
	cnt.add(widget);
	label(lab);
	}

Button::Impl::~Impl()
	{
	m_cb=nullptr;
	gtk_widget_destroy(GTK_WIDGET(m_handle));
	printf("Button %p dtor\n",this);
	}

void Button::Impl::clicked_callback(GtkWidget* widget,gpointer data)
	{
	auto state=reinterpret_cast<Impl*>(data);
	if(state->m_cb!=nullptr)
		{(state->m_cb)(state->m_cb_obj,state->r_owner);}
	}
