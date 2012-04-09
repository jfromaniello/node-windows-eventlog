#pragma comment(lib, "node")

#using <mscorlib.dll>
#using <system.dll>

#include <node.h>
#include <v8.h>
#include <string>
#include <gcroot.h>
#include <string>
#include <iostream>
#include <uv.h>

using namespace node;
using namespace v8;

class EventLog : ObjectWrap
{
private:
	gcroot<System::Diagnostics::EventLog^> _eventLog;

public:

    static Persistent<FunctionTemplate> s_ct;
    static void NODE_EXTERN Init(Handle<Object> target)
    {
        HandleScope scope;

        // set the constructor function
        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        // set the node.js/v8 class name
        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("EventLog"));

        // registers a class member functions 
        NODE_SET_PROTOTYPE_METHOD(s_ct, "log", Log);
        
        target->Set(String::NewSymbol("EventLog"),
            s_ct->GetFunction());
    }

    EventLog(System::String^ source, System::String^ logName) 
    {
		if(!System::Diagnostics::EventLog::SourceExists(source)){
			System::Diagnostics::EventLog::CreateEventSource(source, logName);
		}
        
		_eventLog = gcnew System::Diagnostics::EventLog();
		_eventLog->Source = source;
	}

    ~EventLog()
    {
		delete _eventLog;
	}

	
	static inline gcroot<System::String^> ParseArgument(Arguments const&args, int argumentIndex)
	{
		Local<String> message = Local<String>::Cast(args[argumentIndex]);
		gcroot<System::String^> m = gcnew System::String(((std::string)*v8::String::AsciiValue(message)).c_str());
		return m;
	}

    static Handle<Value> New(const Arguments& args)
    {
		HandleScope scope;

		if (!args[0]->IsString()) {
		    return ThrowException(Exception::TypeError(
		        String::New("First argument must be the name of the event log source")));
		}
		if (!args[1]->IsString()) {
		    return ThrowException(Exception::TypeError(
		        String::New("Second argument must be the name of the event log: (Application, System)")));
		}

		System::String^ s = ParseArgument(args, 0);
		System::String^ ln = ParseArgument(args, 1);

		EventLog* pm = new EventLog(s, ln);

        pm->Wrap(args.This());
        return args.This();
    }


	static Handle<Value> Log(const Arguments& args)
    {
		if (!args[0]->IsString()) {
		    return ThrowException(Exception::TypeError(
		        String::New("First argument must be the message to log.")));
		}
		
		if (!args[1]->IsString()) {
		    return ThrowException(Exception::TypeError(
		        String::New("Second argument must be the type of the entry Information/Warning/Error.")));
		}

		gcroot<System::String^> m = ParseArgument(args, 0);
		gcroot<System::String^> t = ParseArgument(args, 1);
		gcroot<System::Diagnostics::EventLogEntryType> logt = (System::Diagnostics::EventLogEntryType)System::Enum::Parse(System::Diagnostics::EventLogEntryType::typeid, t);

		EventLog* xthis = ObjectWrap::Unwrap<EventLog>(args.This());

		xthis->_eventLog->WriteEntry(m, logt, 1000);

	    return Undefined();
    }
};

Persistent<FunctionTemplate> EventLog::s_ct;

extern "C" {
    void NODE_EXTERN init (Handle<Object> target)
    {
        EventLog::Init(target);
    }
    NODE_MODULE(sharp, init);
}