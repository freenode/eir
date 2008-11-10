#include "bot.h"
#include "command.h"

#include <iostream>
#include <iterator>

using namespace std::tr1::placeholders;

struct notice_printer{
    void print(const eir::Message * m)
    {
        std::cout << m->source << " " << m->command << " " << m->destination << " ";
        std::copy(m->args.begin(), m->args.end(), std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << std::endl;
    }
    notice_printer() {
        _id = eir::CommandRegistry::get_instance()->add_handler("", std::tr1::bind(std::tr1::mem_fn(&notice_printer::print), this, _1));
    }
    ~notice_printer() {
        eir::CommandRegistry::get_instance()->remove_handler(_id);
    }
    eir::CommandRegistry::id _id;
};

notice_printer p;

