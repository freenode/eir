#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct Modloader : public CommandHandlerBase<Modloader>
{
    eir::CommandHolder modload_id, modunload_id, modreload_id;

    void do_modload(const eir::Message *m)
    {
        if(m->source.type != sourceinfo::ConfigFile &&
                !(m->source.client && m->source.client->privs().has_privilege("admin")))
            return;

        Context ctx("Processing MODLOAD " + m->args[0]);

        if (ModuleRegistry::get_instance()->is_loaded(m->args[0]))
        {
            m->source.reply(m->args[0] + " is already loaded.");
        }

        ModuleRegistry::get_instance()->load(m->args[0]);
        m->source.reply("Loaded " + m->args[0]);
    }

    void do_modunload(const eir::Message *m)
    {
        if(m->source.type != sourceinfo::ConfigFile &&
                !(m->source.client && m->source.client->privs().has_privilege("admin")))
            return;

        Context ctx("Processing MODUNLOAD " + m->args[0]);

        ModuleRegistry::get_instance()->unload(m->args[0]);
        m->source.reply("Unloaded " + m->args[0]);
     }

    void do_modreload(const eir::Message *m)
    {
        if(m->source.type != sourceinfo::ConfigFile &&
                !(m->source.client && m->source.client->privs().has_privilege("admin")))
            return;

        Context ctx("Processing MODRELOAD " + m->args[0]);

        if (ModuleRegistry::get_instance()->is_loaded(m->args[0]))
        {
            ModuleRegistry::get_instance()->unload(m->args[0]);
            m->source.reply("Unloaded " + m->args[0]);
        }
        ModuleRegistry::get_instance()->load(m->args[0]);
        m->source.reply("Loaded " + m->args[0]);
     }

    Modloader() {
        modload_id = add_handler("modload", &Modloader::do_modload);
        modunload_id = add_handler("modunload", &Modloader::do_modunload);
        modreload_id = add_handler("modreload", &Modloader::do_modreload);
    }
};

Modloader m;
