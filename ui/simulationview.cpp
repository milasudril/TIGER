//@	 {"targets":[{"name":"simulationview.o","type":"object"}]}

#include "simulationview.hpp"
#include "../engine/simulation.hpp"

using namespace Tiger;

SimulationView::SimulationView(Container& cnt):r_sim(nullptr)
	,m_top(cnt,1)
		,m_toolbar(m_top,0,0)
		,m_lower(m_top.insertMode({2,Box::FILL|Box::EXPAND}),0)
			,m_img_selector(m_lower,1,1)
			,m_img_view(m_lower.insertMode({2,Box::FILL|Box::EXPAND}),0)
	,m_ch_current(0)
	{
	m_toolbar.append("Run").append("Pause").append("Reset").callback(*this);
	}

void SimulationView::clicked(ButtonList<Self>& list,Button& btn)
	{
	if(list.id()==1)
		{
		if(m_ch_current!=-1)
			{
		//	m_img_ranges[m_ch_current]=m_img_view.range();
			list[m_ch_current].state(0);
			}

		auto id=btn.id();
		m_img_view.image(&r_sim->stateCurrent(),id); //.range(m_img_ranges[id]);
		m_ch_current=id;
		btn.state(1);
		}
/*	else
		{
		submit();
		btn.state(0);
		}*/
	}

SimulationView& SimulationView::simulation(Simulation& sim)
	{
	m_img_selector.clear();
	sim.channelsList([this](const Simulation& sim,const char* ch)
		{m_img_selector.append(ch);});
	m_img_selector.callback(*this);
	m_ch_current=-1;
	r_sim=&sim;
	return *this;
	}