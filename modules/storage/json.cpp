#include "eir.h"
#include "storage.h"

#include <functional>
#include <fstream>

#include "json/json.h"

using namespace eir;
using namespace std::tr1::placeholders;

namespace
{
    Json::Value EirValueToJsonValue(const eir::Value &v)
    {
        switch (v.Type())
        {
            case Value::empty:
                return Json::Value::null;

            case Value::integer:
                return Json::Value(v.Int());

            case Value::string:
                return Json::Value(v.String());

            case Value::array:
                {
                    Json::Value ret(Json::arrayValue);
                    for (eir::ValueArray::const_iterator it = v.Array().begin(), it_e = v.Array().end();
                            it != it_e; ++it)
                    {
                        ret.append(EirValueToJsonValue(*it));
                    }
                    return ret;
                }
            case Value::kvarray:
                {
                    Json::Value ret(Json::objectValue);
                    for (eir::KeyValueArray::const_iterator it = v.KV().begin(), it_e = v.KV().end();
                            it != it_e; ++it)
                    {
                        ret[it->first.String()] = EirValueToJsonValue(it->second);
                    }
                    return ret;
                }
            default:
                throw InternalError("Unknown value type when converting to json");
        }
    }

    eir::Value JsonValueToEirValue(const Json::Value &v)
    {
        switch (v.type())
        {
            case Json::nullValue:
                return Value(Value::empty);

            case Json::intValue:
            case Json::uintValue:
            case Json::booleanValue:
                return Value(v.asInt());

            case Json::realValue:
                throw InternalError("Can't represent floating-point numbers");

            case Json::stringValue:
                return Value(v.asString());

            case Json::arrayValue:
                {
                    Value ret(Value::array);
                    for (Json::ValueIterator it = v.begin(); it != v.end(); ++it)
                    {
                        ret.push_back(JsonValueToEirValue(*it));
                    }
                    return ret;
                }

            case Json::objectValue:
                {
                    Value ret(Value::kvarray);
                    Json::Value::Members members(v.getMemberNames());
                    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
                    {
                        ret.KV().insert(*it, JsonValueToEirValue(v.get(*it, Json::Value::null)));
                    }
                    return ret;
                }
            default:
                throw InternalError("Unknown json value type");
        }
    }
}

struct JsonStorage : Module, StorageBackend
{
    void Save(const Value & v, std::string target)
    {
        Json::Value jv = EirValueToJsonValue(v);
        Json::StyledWriter writer;
        std::string filename("data/" + target);
        std::ofstream fs(filename.c_str());
        fs << writer.write(jv);
        if (!fs)
            throw IOError("Error writing json output to " + filename);
    }

    Value Load(std::string source)
    {
        std::string filename("data/" + source);
        std::ifstream fs(filename.c_str());
        Json::Reader reader;
        Json::Value jv;
        std::string raw;

        // Icky hack coming up
        std::stringstream ss;
        ss << fs.rdbuf();
        raw = ss.str();

        if (fs || !ss)
            throw IOError("Error reading from " + filename);

        if (!reader.parse(raw, jv))
            throw IOError("Couldn't parse json input from " + filename);

        return JsonValueToEirValue(jv);
    }

    StorageBackendHolder backendid;

    JsonStorage()
    {
        backendid = StorageManager::get_instance()->register_backend("json", this);
    }
};

MODULE_CLASS(JsonStorage)
