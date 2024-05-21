#include <cstdio>

#ifndef MODULE_NAME
#define MODULE_NAME Launcher
#endif

#include <core/core.h>
#include <plugins/plugins.h>

using namespace WPEFramework;

#define THREADPOOL_COUNT 2

class FactoriesImplementation : public PluginHost::IFactories
{
private:
    FactoriesImplementation(const FactoriesImplementation&) = delete;
    FactoriesImplementation& operator=(const FactoriesImplementation&) = delete;
public:
    FactoriesImplementation()
        : _requestFactory(2)
        , _responseFactory(2)
        , _fileBodyFactory(2)
        , _jsonRPCFactory(2)
    {
    }
    ~FactoriesImplementation() override
    {
    }
public:
    Core::ProxyType<Web::Request> Request() override
    {
        return (_requestFactory.Element());
    }
    Core::ProxyType<Web::Response> Response() override
    {
        return (_responseFactory.Element());
    }
    Core::ProxyType<Web::FileBody> FileBody() override
    {
        return (_fileBodyFactory.Element());
    }
    Core::ProxyType<Web::JSONBodyType<Core::JSONRPC::Message>> JSONRPC() override
    {
        return (_jsonRPCFactory.Element());
    }
private:
    Core::ProxyPoolType<Web::Request> _requestFactory;
    Core::ProxyPoolType<Web::Response> _responseFactory;
    Core::ProxyPoolType<Web::FileBody> _fileBodyFactory;
    Core::ProxyPoolType<Web::JSONBodyType<Core::JSONRPC::Message>> _jsonRPCFactory;
};

struct WorkerPoolDispatcher: public Core::ThreadPool::IDispatcher
{
    void Initialize() { }
    void Deinitialize() { }
#ifdef USE_R2
    void Dispatch(Core::IDispatchType<void>* job)
    {
        job->Dispatch();
    }
#else
    void Dispatch(Core::IDispatch* job)
    {
        job->Dispatch();
    }
#endif
};

class WorkerPoolImplementation : public Core::WorkerPool
{
public:
    WorkerPoolImplementation() = delete;
    WorkerPoolImplementation(const WorkerPoolImplementation&) = delete;
    WorkerPoolImplementation& operator=(const WorkerPoolImplementation&) = delete;
    WorkerPoolImplementation(const uint32_t stackSize, Core::ThreadPool::IDispatcher *dispatcher)
        : Core::WorkerPool(THREADPOOL_COUNT, stackSize, 16, dispatcher)
    {
    }
    virtual ~WorkerPoolImplementation()
    {
    }
};

class DummyService : public PluginHost::Service
{
    struct DummyCOMLink : public PluginHost::IShell::ICOMLink
    {
        void Register(RPC::IRemoteConnection::INotification* sink) { }
        void Unregister(const RPC::IRemoteConnection::INotification* sink) { }
        RPC::IRemoteConnection* RemoteConnection(const uint32_t connectionId)
        {
            return nullptr;
        }
        void* Instantiate(const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId, const string& className, const string& callsign)
        {
            return nullptr;
        }
#ifndef USE_R2
        void Register(INotification* sink)
        {
        }
        void Unregister(INotification* sink)
        {
        }
#endif
        void* Instantiate(const RPC::Object& object, const uint32_t waitTime, uint32_t& connectionId)
        {
            return nullptr;
        }
    };

    DummyCOMLink _dummyCOMLink;

public:

    DummyService(const Plugin::Config& plugin)
        : PluginHost::Service(plugin, "", "", "", "")
    {
    }

    Core::ProxyType<Core::JSON::IElement> Inbound(const string& identifier)
    {
        return {};
    }
    void Notify(const string& message)
    {
    }
    void* QueryInterface(const uint32_t id)
    {
        return nullptr;
    }
    void* QueryInterfaceByCallsign(const uint32_t id, const string& name)
    {
        return nullptr;
    }
    void Register(PluginHost::IPlugin::INotification* sink)
    {
    }
    void Unregister(PluginHost::IPlugin::INotification* sink)
    {
    }
    ICOMLink* COMLink()
    {
        return &_dummyCOMLink;
    }
    uint32_t Activate(const PluginHost::IShell::reason)
    {
        return 0;
    }
    uint32_t Deactivate(const PluginHost::IShell::reason)
    {
        return 0;
    }
    uint32_t Unavailable(const PluginHost::IShell::reason)
    {
        return 0;
    }
    uint32_t Hibernate(const PluginHost::IShell::reason)
    {
        return 0;
    }
    Core::hresult Hibernate(const uint32_t timeout)
    {
        return {};
    }
    Core::hresult Metadata(string& info /* @out */) const
    {
        return {};
    }
    PluginHost::IShell::reason Reason() const
    {
        return PluginHost::IShell::reason::AUTOMATIC;
    }
    string Version() const
    {
        return {};
    }
    string Model() const
    {
        return {};
    }
    bool Background() const
    {
        return {};
    }
    string Accessor() const
    {
        return {};
    }
    string ProxyStubPath() const
    {
        return {};
    }
    string Substitute(const string& input) const
    {
        return {};
    }
    string HashKey() const
    {
        return {};
    }
    PluginHost::ISubSystem* SubSystems()
    {
        return nullptr;
    }
    uint32_t Submit(const uint32_t Id, const Core::ProxyType<Core::JSON::IElement>& response)
    {
        return 0;
    }
    uint8_t Major() const
    {
        return 0;
    }
    uint8_t Minor() const
    {
        return 0;
    }
    uint8_t Patch() const
    {
        return 0;
    }
    string SystemPath() const
    {
        return {};
    }
    string PluginPath() const
    {
        return {};
    }

    std::vector<string> GetLibrarySearchPaths(const string& name) const
    {
        std::vector<string> all_paths;
        all_paths.push_back(name);
        all_paths.push_back("/usr/lib/wpeframework/plugins/" + name);
        return all_paths;
    }

    Core::Library LoadPluginLibrary(const string& name)
    {
        Core::Library result;

        std::vector<string> all_paths = GetLibrarySearchPaths(name);
        for (auto iter = std::begin(all_paths); iter != std::end(all_paths); ++iter)
        {
            Core::File libraryToLoad(*iter);
            if (libraryToLoad.Exists())
            {
                Core::Library newLib = Core::Library(iter->c_str());
                if (newLib.IsLoaded() == true)
                {
                    void* moduleBuildRef = newLib.LoadFunction(_T("ModuleBuildRef"));
                    void* moduleServiceMetadata = newLib.LoadFunction(_T("ModuleServiceMetadata"));
                    if (((moduleBuildRef != nullptr) && (moduleServiceMetadata != nullptr)))
                    {
                        result = newLib;
                        break;
                    }
                }
                fprintf(stderr, "Failed to load: %s\n", iter->c_str());
            }
        }

        return (result);
    }

};

class UpdateCfgFromAgrs : public Core::Options
{
  JsonObject &cfg;
public:
  UpdateCfgFromAgrs(JsonObject &cfg, int argumentCount, TCHAR* arguments[])
    : Core::Options(argumentCount, arguments, "c:")
    , cfg(cfg)
  {
    Parse();
  }

  void UpdateConfig(const string& arg)
  {
    auto i = arg.find('=');
    if (i == std::string::npos)
      return;

    string key = arg.substr(0, i);
    string val = arg.substr(i + 1);

    JsonValue jsonValue;
    if (!jsonValue.IElement::FromString(val)) {
      fprintf(stderr, _T("Failed to parse config option, %s : %s\n"), key.c_str(), val.c_str());
      return;
    }

    cfg.Set(key.c_str(), jsonValue);
  }

  void Option(const TCHAR option, const TCHAR* argument) override
  {
     switch (option)
     {
       case 'c':
         UpdateConfig(argument);
         break;
       default:
         break;
     }
  }
};

static volatile bool gRunning { true };
static void FakeRunLoop()
{
  while (gRunning)
    usleep(16000);
}
static void ExitHandler(int signo)
{
  gRunning = false;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-c 'configkey=configvalue'] <plugin config>\n", argv[0]);
        return -1;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = ExitHandler;
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);

    FactoriesImplementation factories;
    PluginHost::IFactories::Assign(&factories);

    WorkerPoolDispatcher dispatcher;
    WorkerPoolImplementation workerPool(512*1024, &dispatcher);
    Core::WorkerPool::Assign(&workerPool);

    do
    {
      string configFilePath;

      for (int i = argc - 1; i > 0; --i) {
        if (*argv[i] == '-')
          continue;
        configFilePath = argv[i];
        break;
      }

      Plugin::Config pluginCfg;
      Core::OptionalType<Core::JSON::Error> error;
      Core::File configFile(configFilePath);

      if ( !configFile.Open(true) )
      {
        fprintf(stdout, _T("Config file [%s] could not be opened.\n"),
                configFilePath.c_str());
        break;
      }

      if ( !pluginCfg.IElement::FromFile(configFile, error) )
      {
        fprintf(stdout, _T("Config file [%s] parsing failed, error: %s.\n"),
                configFilePath.c_str(),
                ErrorDisplayMessage(error.Value()).c_str());
        break;
      }

      // Update config
      {
        JsonObject cfg;

        if ( !cfg.IElement::FromString(pluginCfg.Configuration.Value(), error) )
        {
          fprintf(stdout, _T("Config file [%s] parsing failed, error: %s.\n"),
                  configFilePath.c_str(),
                  ErrorDisplayMessage(error.Value()).c_str());
          break;
        }

        UpdateCfgFromAgrs(cfg, argc, argv);

        auto root = cfg.Get("root").Object();
        root.Set("mode", "Off");
        cfg.Set("root", root);

        string cfgString;
        cfg.IElement::ToString(cfgString);
        pluginCfg.Configuration = cfgString;

        fprintf(stdout, _T("Updated config:\n%s\n"), pluginCfg.Configuration.Value().c_str());
      }

      auto dummyService = Core::ProxyType<DummyService>::Create(pluginCfg);

      PluginHost::IPlugin* pluginIF = nullptr;
      Core::Library myLib = dummyService->LoadPluginLibrary(string(pluginCfg.Locator.Value()));
      if (myLib.IsLoaded() == true)
      {
        const string classNameString(pluginCfg.ClassName.Value());
        const TCHAR* className(classNameString.c_str());
        uint32_t version(static_cast<uint32_t>(~0));

        if ((pluginIF = Core::ServiceAdministrator::Instance().Instantiate<PluginHost::IPlugin>(myLib, className, version)) == nullptr)
        {
          fprintf(stderr, _T("Class definitions does not exist\n"));
        }
      }
      else
      {
        fprintf(stderr, _T("Failed to load the plugin\n"));
      }

      if (pluginIF)
      {
        pluginIF->Initialize(&(*dummyService));

        PluginHost::IStateControl* stateControl = nullptr;
        if (((stateControl = pluginIF->QueryInterface<PluginHost::IStateControl>()) != nullptr)) {
          stateControl->Request(PluginHost::IStateControl::RESUME);
          stateControl->Release();
        }

        FakeRunLoop();

        fprintf(stderr, _T("\nExec completed!\n"));
        pluginIF->Deinitialize(&(*dummyService));
        pluginIF->Release();

        fprintf(stderr, _T("Plugin IF released!\n"));
      }

    } while (0);

    workerPool.Stop();
    workerPool.Join();

    Core::WorkerPool::Assign(nullptr);
    PluginHost::IFactories::Assign(nullptr);
    Core::Singleton::Dispose();

    return 0;
}

MODULE_NAME_DECLARATION(BUILD_REFERENCE)
