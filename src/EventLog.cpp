#pragma comment(lib, "node")

#using <mscorlib.dll>
#using <system.dll>

#include <node.h>
#include <nan.h>
#include <v8.h>
#include <string>
#include <gcroot.h>
#include <string>
#include <iostream>
#include <vcclr.h>
#include <msclr\marshal_cppstd.h>

class EventLog : Nan::ObjectWrap
{
private:
    gcroot<System::Diagnostics::EventLog^> _eventLog;

public:
    static NAN_MODULE_INIT(Init)
    {
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("EventLog").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        SetPrototypeMethod(tpl, "log", Log);

        constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
        Nan::Set(target, Nan::New("EventLog").ToLocalChecked(),
            Nan::GetFunction(tpl).ToLocalChecked());
    }

    EventLog(System::String^ source, System::String^ logName)
    {
        if (!System::Diagnostics::EventLog::SourceExists(source)) {
            System::Diagnostics::EventLog::CreateEventSource(source, logName);
        }

        _eventLog = gcnew System::Diagnostics::EventLog();
        _eventLog->Source = source;
    }

    ~EventLog()
    {
        delete _eventLog;
    }

    static inline gcroot<System::String^> ParseArgument(const Nan::FunctionCallbackInfo<v8::Value>& args, int argumentIndex)
    {
        Nan::Utf8String arg(args[argumentIndex]->ToString());
        std::string stdArg = *arg;
        std::wstring wideArg(stdArg.begin(), stdArg.end());
        return gcnew System::String(wideArg.c_str());
    }

    static std::string MarshalString(System::String ^ s)
    {
        msclr::interop::marshal_context context;
        return context.marshal_as<std::string>(s);
    }

    static NAN_METHOD(New) {
        if (!info.IsConstructCall()) {
            const int argc = 2;
            v8::Local <v8::Value> argv[argc] = { info[0], info[1] };
            v8::Local <v8::Function> cons = Nan::New(constructor());
            info.GetReturnValue().Set(cons->NewInstance(argc, argv));
            return;
        }

        if (!info[0]->IsString()) {
            Nan::ThrowError("First argument must be the name of the event log source");
            return;
        }

        try
        {
            System::String^ s = ParseArgument(info, 0);
            System::String^ ln;

            if (!info[1]->IsString()) {
                ln = "Application";
            }
            else {
                ln = ParseArgument(info, 1);
            }

            EventLog* pm = new EventLog(s, ln);

            pm->Wrap(info.This());
            info.GetReturnValue().Set(info.This());
        }
        catch (System::Exception^ e)
        {
            Nan::ThrowError(MarshalString(e->Message).c_str());
        }
    }

    static void Log(const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        if (!info[0]->IsString()) {
            Nan::ThrowTypeError("First argument must be the message to log.");
            return;
        }

        gcroot<System::String^> m = ParseArgument(info, 0);
        gcroot<System::String^> t;

        if (!info[1]->IsString()) {
            t = "Information";
        }
        else {
            t = ParseArgument(info, 1);
        }

        gcroot<System::Diagnostics::EventLogEntryType> logt = (System::Diagnostics::EventLogEntryType)System::Enum::Parse(System::Diagnostics::EventLogEntryType::typeid, t);

        EventLog* xthis = ObjectWrap::Unwrap<EventLog>(info.This());

        xthis->_eventLog->WriteEntry(m, logt, 1000);

        //return Undefined();
    }

    static inline Nan::Persistent<v8::Function>& constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
    }

};

extern "C" {
    void init(v8::Handle<v8::Object> target)
    {
        EventLog::Init(target);
    }
    NODE_MODULE(EventLog, init);
}
