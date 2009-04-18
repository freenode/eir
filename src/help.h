#ifndef help_h
#define help_h

#include "settings.h"

namespace eir
{
    struct HelpTopicHolder
    {
        std::string name;
        HelpTopicHolder(std::string n, std::string priv, std::string text)
            :name(n)
        {
            Value& help_root = GlobalSettingsManager::get_instance()->get("help_root");

            if (help_root.Type() != Value::kvarray)
                help_root = Value(Value::kvarray);

            Value help_topic(Value::kvarray);
            help_topic["name"] = name;
            help_topic["priv"] = priv;
            help_topic["text"] = text;

            help_root[name] = help_topic;
        }
        ~HelpTopicHolder()
        {
            Value& help_root = GlobalSettingsManager::get_instance()->get("help_root");
            help_root.KV().erase(Value(name));
        }
    };
}


#endif
