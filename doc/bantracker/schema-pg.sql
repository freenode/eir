CREATE TABLE bans ( i SERIAL PRIMARY KEY, channel VARCHAR(512), setter VARCHAR(512), mask VARCHAR(512), isSet BOOL, setDate TIMESTAMP, reason VARCHAR(512), action INTEGER, actionDate TIMESTAMP, affected TEXT, unbanner VARCHAR(512), unbanDate TIMESTAMP, type VARCHAR(20), nagged INTEGER);
CREATE TABLE log (i SERIAL PRIMARY KEY, channel VARCHAR(512), sender VARCHAR(512), command  VARCHAR(512), data VARCHAR(512), date TIMESTAMP);

