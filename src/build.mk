EXECUTABLES = eir

eir_SOURCES = bot.cpp \
	    bot_command.cpp \
	    capability.cpp \
	    client.cpp \
	    command.cpp \
	    event.cpp \
	    exceptions.cpp \
	    logger.cpp \
	    main.cpp \
	    match.cpp \
	    message.cpp \
	    modload.cpp \
	    modules.cpp \
	    privilege.cpp \
	    server.cpp \
	    settings.cpp \
	    storage.cpp \
	    string_util.cpp \
	    supported.cpp \
	    value.cpp \

eir_LDFLAGS = -Wl,-export-dynamic -Wl,-rpath,$(LIBDIR)
ifeq ($(shell uname),FreeBSD)
	eir_LIBRARIES = paludis/util/paludisutil
else
	eir_LIBRARIES = -ldl paludis/util/paludisutil
endif
