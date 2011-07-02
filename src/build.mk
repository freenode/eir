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
	    supported.cpp \
	    value.cpp \

eir_LDFLAGS = -Wl,-export-dynamic -Wl,-rpath,$(LIBDIR)
eir_LIBRARIES = -ldl paludis/util/paludisutil
