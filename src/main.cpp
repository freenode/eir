/* vim: set sw=4 sts=4 et : */

#include "bot.h"

#include "message.h"
#include "modules.h"
#include "command.h"

#include "exceptions.h"

#include <iostream>

using namespace eir;

void print_cerr(std::string s)
{
    std::cerr << s << std::endl;
}

int main(int, char **argv)
{
    std::string botname("eir");

    if (argv[1] && argv[1][0])
        botname = argv[1];

    Bot bot(botname);

    while (true)
    {
        try
        {
            bot.run();
        }
        catch (DisconnectedException &e)
        {
            std::cerr << "Reconnecting due to error: " << e.message() << std::endl;
            continue;
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
    }

    return 0;
}
