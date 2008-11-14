/* vim: set sw=4 sts=4 et : */

#include "bot.h"

#include "message.h"
#include "modules.h"
#include "command.h"

#include <iostream>
#include <iterator>

void on_connect(const eir::Message *m)
{
    m->bot->send("JOIN #asdfdsa");
}

void print(const eir::Message *m)
{
    std::cerr << m->raw << std::endl;
}

int main()
{
    eir::Bot b("testnet.freenode.net", "9002", "eir", "");
    //eir::CommandRegistry::get_instance()->add_handler("NOTICE", print);
    //eir::ModuleRegistry::get_instance()->load("modules/print_notice.so");
    eir::ModuleRegistry::get_instance()->load("modules/channel.so");
    eir::ModuleRegistry::get_instance()->load("modules/echo.so");
    eir::CommandRegistry::get_instance()->add_handler("001", on_connect);
    eir::CommandRegistry::get_instance()->add_handler("", print);
    b.run();
    return 0;
}
