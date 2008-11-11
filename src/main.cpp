/* vim: set sw=4 sts=4 et : */

#include "bot.h"

#include "message.h"
#include "modules.h"

int main()
{
    eir::Bot b("testnet.freenode.net", "9002", "eir", "");
    //eir::CommandRegistry::get_instance()->add_handler("NOTICE", print);
    eir::ModuleRegistry::get_instance()->load("modules/print_notice.so");
    eir::ModuleRegistry::get_instance()->load("modules/echo.so");
    b.run();
    return 0;
}
