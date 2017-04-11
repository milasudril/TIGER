//@	{"targets":[{"name":"simulation.o","type":"object"}]}

#include "simulation.hpp"
#include "channel.hpp"
#include "parameter.hpp"
#include "srcstdio.hpp"
#include "sinkstdio.hpp"
#include <cassert>

using namespace Tiger;

Simulation::Simulation(const char* filter,const char* objdir):
	m_filter(filter,objdir),m_frame_current(0)
	{}

Simulation::~Simulation()
	{
	}

Simulation& Simulation::run(unsigned long long n_iter) noexcept
	{
	auto k=m_frame_current;
	while(n_iter!=0)
		{
		m_filter.run(m_state,k);
		m_state.swap();
		++k;
		--n_iter;
		}
	m_frame_current=k;
	return *this;
	}

static void imageSoA2AoS(const Tiger::Image& src,unsigned int ch,Tiger::Image& dest)
	{
	assert(src.height()==dest.height() && src.width()==dest.width() 
		&& src.channelCount()==1 && ch<dest.channelCount() );

	auto w=dest.width();
	auto h=dest.height();
	auto N=w*h;
	auto ch_count=dest.channelCount();
	auto pixels_in=src.pixels();
	auto pixels_out=dest.pixels() + ch;
	while(N)
		{
		*pixels_out=*pixels_in;
		pixels_out+=ch_count;
		++pixels_in;
		--N;
		}
	}

Image Simulation::imagesLoad(const std::vector<Tiger::Channel>& files,const Tiger::Filter& f)
	{
	auto ptr=files.begin();
	auto ptr_end=files.end();
	Tiger::Image ret;
	while(ptr!=ptr_end)
		{
		Tiger::Image src{Tiger::SrcStdio(ptr->filename())};
		if(src.channelCount()!=1)
			{throw Tiger::Error(ptr->filename(),": Only grayscale images are supported.");}
		if(!ret.valid())
			{ret=Tiger::Image(src.width(),src.height(),f.channelCount());}
		if( ret.width()!=src.width() || ret.height()!=src.height() )
			{
			throw Tiger::Error(ptr->filename()
				,": Image has a different size comared to other loaded images.");
			}

		auto ch=f.channelIndex(ptr->name());
		imageSoA2AoS(src,ch,ret);
		++ptr;
		}
	return std::move(ret);
	}

static void imageAoS2SoA(const FilterState& src,unsigned int ch_count
	,unsigned int ch,Tiger::Image& dest)
	{
	assert(src.height()==static_cast<int>(dest.height())
		&& src.width()==static_cast<int>(dest.width())
		&& dest.channelCount()==1 && ch<ch_count );;

	auto w=dest.width();
	auto h=dest.height();
	auto N=w*h;
	auto pixels_in=src.buffer_current() + ch;
	auto pixels_out=dest.pixels();
	while(N)
		{
		*pixels_out=*pixels_in;
		pixels_in+=ch_count;
		++pixels_out;
		--N;
		}
	}

void Simulation::imagesStore(const Tiger::Filter& f,const Tiger::FilterState& d
	,const std::vector<Tiger::Channel>& files)
	{
	auto ptr=files.begin();
	auto ptr_end=files.end();
	Tiger::Image img_out(d.width(),d.height(),1);
	while(ptr!=ptr_end)
		{
		auto ch=f.channelIndex(ptr->name());
		imageAoS2SoA(d,f.channelCount(),ch,img_out);
		img_out.store(Tiger::SinkStdio{ptr->filename()});
		++ptr;
		}
	}