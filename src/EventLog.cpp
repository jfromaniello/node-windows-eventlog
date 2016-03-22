#pragma comment(lib, "node")

#include <node.h>
#include <nan.h>
#include <v8.h>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

    static std::string getLastErrorAsString() {
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return std::string();
        }

        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        DWORD languageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
        LPSTR messageBuffer = NULL;
        size_t size = FormatMessageA(flags, NULL, errorMessageID, languageId, (LPSTR)&messageBuffer, 0, NULL);

        std::string message(messageBuffer, size);
        LocalFree(messageBuffer);
        return message;
    }

    static bool parseSeverity(const std::string& severity, WORD *type) {
        if (severity == "info") {
            *type = EVENTLOG_INFORMATION_TYPE;
            return true;
        }
        else if (severity == "warn" || severity == "warning") {
            *type = EVENTLOG_WARNING_TYPE;
            return true;
        }
        else if (severity == "error") {
            *type = EVENTLOG_ERROR_TYPE;
            return true;
        }
        else {
            return false;
        }
    }

    static bool logSyncImpl(HANDLE hdl, DWORD eventId, WORD type, const std::string& message) {
        WORD category = 0; // no category
        PSID user = NULL; // use process' user
        DWORD binDataSize = 0; // no additional binary data provided
        LPVOID binData = NULL;
        const WORD numStrings = 1; // we just log the provided message
        LPCTSTR strings[numStrings];
        std::wstring wideMessage(message.begin(), message.end());
        strings[0] = wideMessage.c_str();
        return ReportEvent(hdl, type, category, eventId, user, numStrings, binDataSize, strings, binData);
    }

    class EventLogAsync : public Nan::AsyncWorker {
    public:
        EventLogAsync(Nan::Callback *callback, HANDLE handle, DWORD eventId, WORD type, const std::string& message)
            : Nan::AsyncWorker(callback), handle(handle), eventId(eventId), type(type), message(message) { }

        virtual void Execute() {
            if (!logSyncImpl(handle, eventId, type, message)) {
                SetErrorMessage(getLastErrorAsString().c_str());
            }
        }

        void HandleErrorCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> argv[] = { Nan::Error(ErrorMessage()) };
            callback->Call(1, argv);
        }

        void HandleOKCallback() {
            Nan::HandleScope scope;
            v8::Local <v8::Value> argv[] = { Nan::Null() };
            callback->Call(1, argv);
        }

    private:
        HANDLE handle;
        DWORD eventId;
        WORD type;
        std::string message;
    };

    class EventLog : public Nan::ObjectWrap {
    public:

        static NAN_MODULE_INIT(Init) {
            v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
            tpl->SetClassName(Nan::New("EventLog").ToLocalChecked());
            tpl->InstanceTemplate()->SetInternalFieldCount(1);

            SetPrototypeMethod(tpl, "log", logAsync);
            SetPrototypeMethod(tpl, "logSync", logSync);

            constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
            Nan::Set(target, Nan::New("EventLog").ToLocalChecked(),
                Nan::GetFunction(tpl).ToLocalChecked());
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

            Nan::Utf8String source(info[0]->ToString());

            EventLog* eventLog = new EventLog(*source);
            eventLog->Wrap(info.This());
            info.GetReturnValue().Set(info.This());
        }

        EventLog(const std::string& source) {
            std::wstring wideSource(source.begin(), source.end());
            eventLogHandle_ = RegisterEventSource(NULL, wideSource.c_str());
            if (!eventLogHandle_) {
                Nan::ThrowError(("Unable to register event source: " + getLastErrorAsString()).c_str());
            }
        }

        ~EventLog() {
            DeregisterEventSource(eventLogHandle_);
        }

        static NAN_METHOD(logAsync) {
            if (!(info[0]->IsString() && info[1]->IsFunction()) &&
                !(info[0]->IsString() && info[1]->IsString() && info[2]->IsFunction())) {
                Nan::ThrowError("A message and callback must be provided.");
                return;
            }

            bool severityProvided = info[1]->IsString();
            std::string severity = severityProvided ? *Nan::Utf8String(info[0]->ToString()) : "info";
            std::string message = *Nan::Utf8String(info[severityProvided ? 1 : 0]->ToString());
            Nan::Callback *callback = new Nan::Callback(info[severityProvided ? 2 : 1].As<v8::Function>());

            WORD type;
            if (!parseSeverity(severity, &type)) {
                Nan::ThrowError(("Unsupported severity " + severity).c_str());
                return;
            }

            DWORD eventId = 1000; // TODO: allow user to change event id.
            EventLog* eventLog = Nan::ObjectWrap::Unwrap<EventLog>(info.Holder());
            Nan::AsyncQueueWorker(new EventLogAsync(callback, eventLog->eventLogHandle_, eventId, type, message));
        }

        static NAN_METHOD(logSync) {
            if (!(info[0]->IsString() && info[1]->IsString()) &&
                !(info[0]->IsString() && info[1]->IsUndefined())) {
                Nan::ThrowError("A message and an optional severity must be provided.");
                return;
            }

            bool severityProvided = info[1]->IsString();
            std::string severity = severityProvided ? *Nan::Utf8String(info[0]->ToString()) : "info";
            std::string message = *Nan::Utf8String(info[severityProvided ? 1 : 0]->ToString());

            WORD type;
            if (!parseSeverity(severity, &type)) {
                Nan::ThrowError(("Unsupported severity " + severity).c_str());
                return;
            }

            DWORD eventId = 1000; // TODO: allow user to change event id.
            EventLog* eventLog = Nan::ObjectWrap::Unwrap<EventLog>(info.Holder());
            if (!logSyncImpl(eventLog->eventLogHandle_, eventId, type, message)) {
                Nan::ThrowError(("Error while logging " + getLastErrorAsString()).c_str());
                return;
            }

            info.GetReturnValue().Set(true);
        }

        static inline Nan::Persistent<v8::Function> & constructor() {
            static Nan::Persistent<v8::Function> my_constructor;
            return my_constructor;
        }

        HANDLE eventLogHandle_;
    };

} // anonymous namespace

NODE_MODULE(EventLog, EventLog::Init);
