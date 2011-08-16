CREATE TABLE bans ( i INTEGER PRIMARY KEY, channel VARCHAR, setter VARCHAR, mask VARCHAR, isSet BOOL, setDate DATETIME, reason VARCHAR, action INTEGER, actionDate DATETIME, "affected" TEXT, unbanner TEXT, unbanDate DATETIME, type varchar, nagged integer);
CREATE TABLE log (i INTEGER PRIMARY KEY, channel VARCHAR, operator VARCHAR, mask VARCHAR, action VARCHAR, date DATETIME);
