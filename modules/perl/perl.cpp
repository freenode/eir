#include "stupid_perl_workarounds.h"

#include "eir.h"
#include "handler.h"

#include <EXTERN.h>
#include <perl.h>

#include <dlfcn.h>

#include "definitions.h"

#include <paludis/util/join.hh>

// Include the xs_init definition here, as i'm too lazy to make the build system
// include two source files for this .so
extern "C" {
#include "perlxsi.c"
}

using namespace eir;

#define TYPE_MAPS_LIGHT
#include "util/type_maps.h"
#include "util/call_perl.hh"

struct PerlModule : CommandHandlerBase<PerlModule>, Module
{
    PerlInterpreter *my_perl;
    void *libperl_handle;

    void do_script_load(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("I need a file name to load.");
            return;
        }
        call_perl<PerlContext::Void>(aTHX_ "Eir::Init::load_script", m->args[0]);
        m->source.reply("Successfully loaded " + m->args[0]);
    }

    void do_script_unload(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("I need a file name to unload.");
            return;
        }
        call_perl<PerlContext::Void>(aTHX_ "Eir::Init::unload_script", m->args[0], m);
        m->source.reply("Successfully unloaded " + m->args[0]);
    }

    void do_script_exec(const Message *m)
    {
        if (m->args.empty())
        {
            m->source.error("I can't execute nothing.");
            return;
        }
        call_perl<PerlContext::Void>(aTHX_ "Eir::Init::do_eval", 
                                        paludis::join(m->args.begin(), m->args.end()," "), m);
        m->source.reply("Done.");
    }

    void startup()
    {
        // Hack alert: Perl extension .sos aren't linked against libperl; they
        // rely on libperl already being loaded when they are.
        // However, this doesn't work for eir because this module, which is what
        // brought in libperl, was loaded with RTLD_LOCAL.
        // So, we re-open libperl with RTLD_GLOBAL to make its symbols visible.
        if (!(libperl_handle = dlopen("libperl.so", RTLD_GLOBAL | RTLD_LAZY | RTLD_NOLOAD)))
            throw InternalError("Couldn't re-open libperl.so");

        const char *_perl_argv[] = { "", PERL_INIT_FILE, };
        char **perl_argv = const_cast<char**>(_perl_argv);
        int perl_argc = 2;
        char **env = NULL;
        PERL_SYS_INIT3(&perl_argc, &perl_argv, &env);

        my_perl = perl_alloc();
        if (!my_perl)
            throw InternalError("Couldn't create a perl interpreter.");

        perl_construct(my_perl);

        PL_origalen = 1;
        int exitstatus = perl_parse(my_perl, xs_init, perl_argc, perl_argv, NULL);
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

        if (exitstatus != 0)
            throw InternalError("Couldn't start the perl interpreter.");

        perl_run(my_perl);
    }

    void shutdown()
    {
        PL_perl_destruct_level = 0;
        perl_destruct(my_perl);
        perl_free(my_perl);
        my_perl = 0;
        PERL_SYS_TERM();

        if (libperl_handle)
            dlclose(libperl_handle);
    }

    CommandHolder load_id, unload_id, exec_id;

    PerlModule()
        : my_perl(0)
    {
        startup();
        load_id = add_handler(filter_command_privilege("loadscript", "admin").or_config(),
                &PerlModule::do_script_load);
        unload_id = add_handler(filter_command_privilege("unloadscript", "admin").or_config(),
                &PerlModule::do_script_unload);
        exec_id = add_handler(filter_command_privilege("execscript", "admin"),
                &PerlModule::do_script_exec);
    }

    ~PerlModule()
    {
        shutdown();
    }
};

MODULE_CLASS(PerlModule)



