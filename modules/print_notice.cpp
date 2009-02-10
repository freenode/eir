#include "bot.h"
#include "handler.h"

#include <iostream>
#include <iterator>

using namespace std::tr1::placeholders;
using namespace eir;

struct NoticePrinter : public CommandHandlerBase<NoticePrinter>
{
    void print(const eir::Message * m)
    {
        std::cout << m->raw << std::endl;
    }
    NoticePrinter() {
        _id = add_handler("server_incoming", &NoticePrinter::print);
    }
    ~NoticePrinter() {
        remove_handler(_id);
    }
    eir::CommandRegistry::id _id;
};

NoticePrinter p;

