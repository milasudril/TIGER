//@	{
//@	"targets":
//@		[{
//@		 "name":"filter.o","type":"object"
//@		,"pkgconfig_libs":["uuid"]
//@		,"dependencies":[{"ref":"magic","rel":"external"}]
//@		}]
//@	}

#include "filter.hpp"
#include "error.hpp"
#include "blob.hpp"
#include "sinkstdio.hpp"
#include <magic.h>
#include <uuid.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <stack>
#include <array>


using namespace Tiger;

inline bool begins_with(const char* str,const char* str2)
	{return strncmp(str,str2,strlen(str2))==0;}

TIGER_BLOB(char,host,"host.hpp");
TIGER_BLOB(char,client,"client.hpp");

static std::array<char,37> uuid_generate() noexcept
	{
	std::array<char,37> ret;
	uuid_t id;
	::uuid_generate(id);
	uuid_unparse_upper(id,ret.begin());
	return ret;
	}

namespace
	{
	class MagicHandle
		{
		public:
			MagicHandle(const MagicHandle&)=delete;
			MagicHandle& operator=(const MagicHandle&)=delete;

			MagicHandle():m_handle(magic_open(MAGIC_ERROR|MAGIC_MIME))
				{magic_load(m_handle,nullptr);}
			~MagicHandle()
				{magic_close(m_handle);}

			const char* identify(const char* filename) noexcept
				{return magic_file(m_handle,filename);}

		private:
			magic_t m_handle;
		};

	class DirectoryGuard
		{
		public:
			DirectoryGuard& operator=(const DirectoryGuard&)=delete;
			DirectoryGuard(const DirectoryGuard&)=delete;

			DirectoryGuard(const std::string& dir):m_dir(dir)
				{mkdir(m_dir.c_str(),S_IRWXU);}

			~DirectoryGuard()
				{	
				while(!m_items.empty())
					{
					auto x=m_items.top();
					m_items.pop();
					remove(x.c_str());
					}
				rmdir(m_dir.c_str());
				}

			DirectoryGuard& itemAdd(const char* name)
				{
				m_items.push(m_dir + '/' + name);
				return *this;
				}

		private:
			std::stack<std::string> m_items;
			std::string m_dir;
		};
	}

static void compile(const char* src,const char* dest)
	{
	auto tmp=std::string("/dev/shm/");
	tmp+=uuid_generate().data();
	DirectoryGuard g(tmp);
	printRange(host_begin,host_end,SinkStdio((tmp+"/host.hpp").c_str()));
	g.itemAdd("host.hpp");
	printRange(client_begin,client_end,SinkStdio((tmp+"/client.hpp").c_str()));
	g.itemAdd("client.hpp");
	
//TODO: Use libmaike (somewhat overkill here) or fork/exec pair
	std::string cmdbuff("g++ -std=c++14 -O3 -iquote'");
	cmdbuff+=tmp;
	cmdbuff+="' -include client.hpp --fast-math -march=native -fno-stack-protector "
		"-fpic -shared -o '";
	cmdbuff+=dest;
	cmdbuff+="' '";
	cmdbuff+=src;
	cmdbuff+="'";

	if(system(cmdbuff.c_str())!=0)
		{throw Error("It was not possible to compile filter ",src);}
	}

static std::string objectGenerate(const char* src,const char* objdir)
	{
	MagicHandle m;
	auto mime=m.identify(src);
	if(mime==nullptr)
		{throw Error(src," has unknown MIME type");}
	if(begins_with(mime,"text/x-c") || begins_with(mime,"text/plain"))
		{
		std::string ret(objdir);
		ret+='/';
		ret+=src;
		ret+=".so";
		compile(src,ret.c_str());
		return ret;
		}
	if(begins_with(mime,"application/x-sharedlib"))
		{return std::string(src);}
	throw Error("Tiger cannot use a filter of type ",mime);
	}

static void lookup_fill(const char* const* names
	,std::map<std::string,unsigned int>& index_map)
	{
	if(names==nullptr)
		{return;}
	auto N=0;
	while((*names)!=nullptr)
		{
		auto ip=index_map.insert({*names,N});
		if(!ip.second)
			{throw Error(*names," is already used within this context");}
		++names;
		++N;
		}
	}

Filter::Filter(const char* src,const char* objdir):Plugin(objectGenerate(src,objdir))
	{
		{
		auto fn=entryPoint<decltype(&::parameters)>("_Z10parametersv");
		lookup_fill(fn(),m_param_index);
		m_params.resize(m_param_index.size());
		}

		{
		auto fn=entryPoint<decltype(&channels)>("_Z8channelsv");
		lookup_fill(fn(),m_channel_index);
		}

	m_process=entryPoint<decltype(m_process)>("_ZN5Tiger9__processERKNS_11ProcessDataE");
	}

void Filter::paramSet(const Parameter& param)
	{
	auto i=m_param_index.find(param.name());
	if(i==m_param_index.end())
		{throw Error(name()," does not take a parameter with name ",param.name());}
	m_params[i->second]=param.value();
	}

void Filter::paramsList(FILE* output) const
	{
	auto ptr=m_param_index.begin();
	auto ptr_end=m_param_index.end();
	fprintf(output,"\nParameters\n----------\n");
	while(ptr!=ptr_end)
		{
		fprintf(output," * `%s`=%.9g\n",ptr->first.c_str(),m_params[ptr->second]);
		++ptr;
		}
	}

void Filter::channelsList(FILE* output) const
	{
	auto ptr=m_channel_index.begin();
	auto ptr_end=m_channel_index.end();
	fprintf(output,"\nChannels\n--------\n");
	while(ptr!=ptr_end)
		{
		fprintf(output," * `%s`\n",ptr->first.c_str());
		++ptr;
		}
	}

unsigned int Filter::channelIndex(const std::string& ch) const
	{
	auto i=m_channel_index.find(ch);
	if(i==m_channel_index.end())
		{throw Error(name()," does use a channel with name ",ch.c_str());}
	return i->second;
	}

unsigned int Filter::channelCount() const noexcept
	{return m_channel_index.size();}
