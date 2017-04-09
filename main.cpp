//@	{
//@	"targets":
//@		[{
//@		 "name":"tiger","type":"application"
//@		,"dependencies":[{"ref":"alice","rel":"external"}]
//@		}]
//@	}

#include "filter.hpp"
#include "parameter.hpp"
#include "channel.hpp"
#include "error.hpp"
#include "image.hpp"
#include "srcstdio.hpp"
#include "sdk.hpp"
#include <alice/alice.hpp>
#include <cassert>
#include <cstdio>

namespace Alice
	{
	template<>
	struct MakeType<Stringkey("Parameter")>
		{
		typedef Tiger::Parameter Type;
		static constexpr const char* descriptionShortGet() noexcept
			{return "Name:Value";}
		static constexpr const char* descriptionLongGet() noexcept
			{
			return "Parameters control the algorithm used by *filter*. "
				"Use --params-list to see a list of parameters availible.";
			}
		};

	template<class ErrorHandler>
	struct MakeValue<Tiger::Parameter,ErrorHandler>
		{
		static Tiger::Parameter make_value(const std::string& str);
		};

	template<class ErrorHandler>
	Tiger::Parameter MakeValue<Tiger::Parameter,ErrorHandler>::make_value(const std::string& str)
		{return Tiger::Parameter(str.c_str());}



	template<>
	struct MakeType<Stringkey("Channel mapping")>
		{
		typedef Tiger::Channel Type;
		static constexpr const char* descriptionShortGet() noexcept
			{return "Channel:Filename";}
		static constexpr const char* descriptionLongGet() noexcept
			{
			return "A Channel mapping maps a channel to a given file. Use "
				"--channels-list to see a list of availible channels.";
			}
		};

	template<class ErrorHandler>
	struct MakeValue<Tiger::Channel,ErrorHandler>
		{
		static Tiger::Channel make_value(const std::string& str);
		};

	template<class ErrorHandler>
	Tiger::Channel MakeValue<Tiger::Channel,ErrorHandler>::make_value(const std::string& str)
		{return Tiger::Channel(str.c_str());}
	}

ALICE_OPTION_DESCRIPTOR(OptionDescriptor
	,{"","help","Print usage information","string",Alice::Option::Multiplicity::ZERO_OR_ONE}
	,{"","filter","Name of filter","string",Alice::Option::Multiplicity::ONE}
	,{"","params","Filter parameters","Parameter",Alice::Option::Multiplicity::ZERO_OR_MORE}
	,{"","params-list","Lists all parameters availible for *filter*","string",Alice::Option::Multiplicity::ZERO_OR_ONE}
	,{"","channels-list","Lists all channel names used by *filter","string",Alice::Option::Multiplicity::ZERO_OR_ONE}
	,{"","source","Selects static source image","Channel mapping",Alice::Option::Multiplicity::ONE_OR_MORE}
	,{"","init","Selects initial image","Channel mapping",Alice::Option::Multiplicity::ONE_OR_MORE}
	,{"","dest","Selects output image","Channel mapping",Alice::Option::Multiplicity::ONE_OR_MORE}
	,{"","iterations","Selects the number of iterations to apply the filter","unsigned int",Alice::Option::Multiplicity::ONE}
	);

static void params_set(const std::vector<Tiger::Parameter>& params,Tiger::Filter& filter)
	{
	auto ptr=params.begin();
	auto ptr_end=params.end();
	while(ptr!=ptr_end)
		{
		filter.paramSet(*ptr);
		++ptr;
		}
	}

void imageFill(const Tiger::Image& src,unsigned int ch,Tiger::Image& dest)
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

Tiger::Image imagesLoad(const std::vector<Tiger::Channel>& files,const Tiger::Filter& f)
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
		imageFill(src,ch,ret);
		++ptr;
		}
	return std::move(ret);
	}

void imagesStore(const std::vector<Tiger::Channel>& files)
	{
	auto ptr=files.begin();
	auto ptr_end=files.end();
	while(ptr!=ptr_end)
		{
		fprintf(stderr,"%s->%s\n",ptr->filename(),ptr->name());
		++ptr;
		}
	}


static void simulation_run(unsigned int N,const Tiger::Filter& filter
	,Tiger::ProcessData& data)
	{
	while(N!=0)
		{
		filter.run(data);
		data.swap();
		--N;
		}
	}

int main(int argc,char** argv)
	{
	try
		{
		Alice::CommandLine<OptionDescriptor> cmdline{argc,argv};

		if(cmdline.get<Alice::Stringkey("help")>())
			{
			cmdline.help(0);
			return 0;
			}

		Tiger::Filter filter(cmdline.get<Alice::Stringkey("filter")>().valueGet().c_str());
		params_set(cmdline.get<Alice::Stringkey("params")>().valueGet(),filter);
		if(cmdline.get<Alice::Stringkey("params-list")>())
			{
		//TODO: If set, use argument value as filename
			filter.paramsList(stdout);
			}

		if(cmdline.get<Alice::Stringkey("channels-list")>())
			{
		//TODO: If set, use argument value as filename
			filter.channelsList(stdout);
			}

		if(!(cmdline.get<Alice::Stringkey("source")>()
			|| cmdline.get<Alice::Stringkey("init")>()
			|| cmdline.get<Alice::Stringkey("dest")>() ) )
			{return 0;}

		if(!cmdline.get<Alice::Stringkey("source")>())
			{throw Tiger::Error("Source files are missing");}

		if(!cmdline.get<Alice::Stringkey("init")>())
			{throw Tiger::Error("Initial condition is missing");}

		if(!cmdline.get<Alice::Stringkey("dest")>())
			{throw Tiger::Error("List of destination files is missing");}

		const auto img_src=imagesLoad(cmdline.get<Alice::Stringkey("source")>().valueGet(),filter);
		auto img_init=imagesLoad(cmdline.get<Alice::Stringkey("init")>().valueGet(),filter);

		if(!layoutSame(img_src,img_init))
			{throw Tiger::Error("Source image and initial image must have the same size.");}

		auto img_next=layoutClone(img_init);

		Tiger::ProcessData procdata
			{
			 img_next.pixels()
			,img_init.pixels()
			,img_src.pixels()
			,filter.parameters()
			,static_cast<int>( img_init.width() )
			,static_cast<int>( img_init.height() )
			};

		simulation_run(cmdline.get<Alice::Stringkey("iterations")>().valueGet()
			,filter,procdata);

		imagesStore(cmdline.get<Alice::Stringkey("dest")>().valueGet());
		}
	catch(const Alice::ErrorMessage& message)
		{
		fprintf(stderr,"%s\n",message.data);
		return -1;
		}
	catch(const Tiger::Error& e)
		{
		fprintf(stderr,"%s\n",e.message());
		return -1;
		}
	return 0;
	}
