#include "bot.h"
#include "modules.h"
#include "handler.h"

#include <iostream>
#include <iterator>

using namespace std::tr1::placeholders;
using namespace eir;

struct NoticePrinter : public CommandHandlerBase<NoticePrinter>, public Module
{
    void print(const eir::Message * m)
    {
        std::cout << m->raw << std::endl;
    }
    NoticePrinter() {
        _id = add_handler(filter_command_type("server_incoming", sourceinfo::Internal), &NoticePrinter::print);
    }
    CommandHolder _id;
};

MODULE_CLASS(NoticePrinter)

