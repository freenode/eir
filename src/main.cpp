/* vim: set sw=4 sts=4 et : */

#include "bot.h"

#include <iostream>
#include <iterator>

void print(const eir::Message & m)
{
    std::cout << m.source << " " << m.command << " " << m.destination << " ";
    std::copy(m.args.begin(), m.args.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;
}

int main()
{
    eir::Bot b("testnet.freenode.net", "9002", "eir", "eir");
    b.dispatcher()->add_handler("NOTICE", print);
    b.run();
    return 0;
}
