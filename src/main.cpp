/* vim: set sw=4 sts=4 et : */

#include "bot.h"

#include "message.h"
#include "modules.h"
#include "command.h"

#include "exceptions.h"

#include <iostream>
#include <iterator>

#include <fstream>
#include <paludis/util/tokeniser.hh>

using namespace eir;

void print_cerr(std::string s)
{
    std::cerr << s << std::endl;
}

static Bot *bot = 0;

static void set_servername(const Message *m)
{
    std::string host, port, nick, pass;

    if(bot)
    {
        m->source.reply("Can't create more than one bot.");
        return;
    }

    if(m->args.size() < 3)
    {
        m->source.reply("Not enough arguments to set_servername. Need 3 or 4.");
        return;
    }

    host = m->args[0];
    port = m->args[1];
    nick = m->args[2];

    if(m->args.size() > 3)
        pass = m->args[3];

    bot = new Bot(host, port, nick, pass);
}

int main(int, char **argv)
{
    std::ifstream fs("eir.conf");
    std::string line;

    eir::CommandRegistry::get_instance()->add_handler("server", set_servername);

    try
    {
        while(std::getline(fs, line))
        {
            std::list<std::string> tokens;
            paludis::tokenise_whitespace_quoted(line, std::back_inserter(tokens));

            if(tokens.empty())
                continue;

            Message m(0, *tokens.begin());

            tokens.pop_front();
            std::copy(tokens.begin(), tokens.end(), std::back_inserter(m.args));

            m.source.reply_func = print_cerr;

            m.source.type = sourceinfo::ConfigFile;

            m.raw = line;

            CommandRegistry::get_instance()->dispatch(&m);
        }
    }
    catch(paludis::Exception & e)
    {
        std::cerr << "Error loading config file:" << std::endl
                  << e.backtrace("\n  * ")
                  << e.message() << " (" << e.what() << ")" << std::endl;
        return 1;
    }
    if (! bot)
    {
        std::cerr << "You didn't create a bot. Bad you." << std::endl;
        return 1;
    }

    try
    {
        bot->run();
    }
    catch (RestartException &e)
    {
        execv(argv[0], argv);
    }
    catch (DieException &e)
    {
        std::cerr << "Shutting down. " << e.message() << std::endl;
        return 0;
    }
    catch (paludis::Exception & e)
    {
        std::cerr << "Aborting due to exception:" << std::endl
                  << e.backtrace("\n  * ")
                  << e.message() << " (" << e.what() << ")" << std::endl;
        return 1;
    }

    return 0;
}
