#include <Base.h>
#include <Tabby.h>

namespace App {

class Sandbox : public Tabby::Application {
public:
    Sandbox(const Tabby::ApplicationSpecification& specification)
        : Tabby::Application(specification)
    {
        PushLayer(new Base());
    }

    ~Sandbox() { }
};

}

Tabby::Application* Tabby::CreateApplication(Tabby::ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name = "A winfow";
    spec.WorkingDirectory = "assets";
    spec.CommandLineArgs = args;
    spec.Width = 1440;
    spec.Height = 900;
    spec.MinWidth = 198;
    spec.MinHeight = 108;
    spec.VSync = false;
    spec.MaxFPS = 120.0f;
    spec.RendererAPI = ApplicationSpecification::RendererAPI::OpenGLES3;

    return new App::Sandbox(spec);
}

#include <Tabby/Core/EntryPoint.h>
