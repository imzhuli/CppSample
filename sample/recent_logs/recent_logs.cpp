#include "./type.hpp"

static constexpr const char * DefaultRequestPath = "/log100/post";

static std::string        Path;
static xIoContext         IoContext;
static xNetAddress        CheckHttpServerAddress;
static xCheckHttpServer   CheckHttpServer;

int main(int argc, char *argv[])
{
	xCommandLine Cmd{argc, argv, {
		{ 'p', "path", "path", true },
		{ 'a', "address", "address", true},
	}};

	Path = DefaultRequestPath;
	auto OptPath = Cmd["path"];
	if (OptPath()) {
		Path = *OptPath;
	}
	cout << "Path: " << Path << endl;

	CheckHttpServerAddress = xNetAddress::Parse("0.0.0.0:10080");
	auto OptAddress = Cmd["address"];
	if (OptAddress()) {
		CheckHttpServerAddress = xNetAddress::Parse(*OptAddress);
	}
	cout << "Address: " << CheckHttpServerAddress.ToString() << endl;

	if (!IoContext.Init()) {
		cerr << "Failed to init iocontext" << endl;
		return -1;
	}
	xScopeGuard IoContextCleaner = []{
		IoContext.Clean();
	};

    if (!CheckHttpServer.Init(&IoContext, CheckHttpServerAddress)) {
        cerr << "Failed to initialize check http server" << endl;
        return -1;
    }
    xScopeGuard HttpServerCleaner = []{CheckHttpServer.Clean();};

	CheckHttpServer.SetLogRequestPath(Path);
	while(true) {
        IoContext.LoopOnce(100);
        CheckHttpServer.Shrink();
	}

	return 0;
}