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
//@	 {"targets":[{"name":"simulationeditor.o","type":"object"}]}

#include "simulationeditor.hpp"
#include "window.hpp"
#include "imageview.hpp"
#include "filenameselect.hpp"
#include "../engine/srcstdio.hpp"
#include "../engine/simulation.hpp"
#include "../engine/mimeidentifier.hpp"
#include <algorithm>

using namespace Tiger;

constexpr SimulationEditor::ParamDataDescriptor SimulationEditor::s_desc;

SimulationEditor::SimulationEditor(Container& cnt,int id):m_id(id),r_sim(nullptr)
	,m_top(cnt,0)
		,m_left(m_top.insertMode({2,Box::FILL|Box::EXPAND}),1)
			,m_init_label(m_left,"Initial conditions")
			,m_init_panels(m_left.insertMode({4,Box::FILL|Box::EXPAND}),0)
				,m_init_list(m_init_panels,0,1)
				,m_img_view(m_init_panels.insertMode({2,Box::FILL|Box::EXPAND}),0)
		,m_sep(m_top.insertMode({2,0}),1)
		,m_right(m_top.insertMode({2,Box::FILL|Box::EXPAND}),1)
			,m_param_label(m_right,"Parameters")
			,m_params(m_right.insertMode({0,Box::FILL|Box::EXPAND}),0,s_desc)
	{
	m_init_list.callback(*this);
	m_params.callback(*this);
	m_img_view.displayCallback(*this);
	m_ch_current=-1;
	}

void SimulationEditor::clicked(ButtonList<SimulationEditor>& list,Button& btn)
	{
	if(m_ch_current!=-1)
		{
		m_img_ranges[m_ch_current]=m_img_view.range();
		list[m_ch_current].state(0);
		}
	if(btn.id()<r_sim->channelCount())
		{
		auto id=btn.id();
		m_img_view.image(&m_img_staged[id],0).range(m_img_ranges[id]);
		m_ch_current=id;
		}
	else
		{
		m_ch_current=-1;
		btn.state(0);
		m_img_view.image(nullptr,0);
		}
	}

void SimulationEditor::itemChanged(MapView<ParamDataDescriptor>& params,float& param
	,const char* valstr)
	{
	param=strtof(valstr,nullptr);
	}

void SimulationEditor::clicked(ImageDisplay& src)
	{
	auto ch_current=m_ch_current;
	if(ch_current!=-1)
		{
		MimeIdentifier ident;
		if(filenameSelect(m_top,m_img_names[ch_current],FileselectMode::OPEN
			,[&ident](const char* filename)
				{return begins_with(ident.identify(filename),"image/png");}
			,"PNG image files"))
			{
		//TODO: try/catch errors
			SrcStdio src(m_img_names[ch_current].c_str());
			m_img_staged[ch_current]=Image(src);
			m_img_view.image(&m_img_staged[ch_current],0);
			auto r=m_img_staged[ch_current].range(0);
			m_img_view.range(r);
			m_img_ranges[ch_current]=r;
			}
		}
	}

SimulationEditor& SimulationEditor::simulation(Simulation& sim)
	{
	m_params.clear();
	sim.paramsList([this](const Simulation& sim,const char* const& name
		,float& value)
		{m_params.recordAppend(name,value);});

	r_sim=&sim;
	m_init_list.clear();
	sim.channelsList([this](const Simulation& sim,const char* ch)
		{m_init_list.append(ch);});
	m_init_list.append("Load all");
	sim.imagesStore(m_img_staged);
	m_img_names.resize(m_img_staged.size());
	m_img_ranges.clear();
	std::for_each(m_img_staged.begin(),m_img_staged.end()
		,[this](const Image& img)
			{m_img_ranges.push_back(img.range(0));});
	m_ch_current=-1;
	return *this;
	}
