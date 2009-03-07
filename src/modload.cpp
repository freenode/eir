#include "eir.h"

#include "handler.h"

using namespace std::tr1::placeholders;
using namespace eir;

struct Modloader : public CommandHandlerBase<Modloader>
{
    eir::CommandHolder modload_id, modunload_id, modreload_id;

    void do_modload(const eir::Message *m)
    {
        if (ModuleRegistry::get_instance()->is_loaded(m->args[0]))
        {
            m->source.reply(m->args[0] + " is already loaded.");
        }

        ModuleRegistry::get_instance()->load(m->args[0]);
        m->source.reply("Loaded " + m->args[0]);
    }

    void do_modunload(const eir::Message *m)
    {
        Context ctx("Processing MODUNLOAD " + m->args[0]);

        ModuleRegistry::get_instance()->unload(m->args[0]);
        m->source.reply("Unloaded " + m->args[0]);
     }

    void do_modreload(const eir::Message *m)
    {
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
        modload_id   = add_handler(filter_command("modload").requires_privilege("admin").or_config(),
                                    &Modloader::do_modload);
        modunload_id = add_handler(filter_command("modunload").requires_privilege("admin").or_config(),
                                    &Modloader::do_modunload);
        modreload_id = add_handler(filter_command("modreload").requires_privilege("admin").or_config(),
                                    &Modloader::do_modreload);
    }
};

Modloader m;
