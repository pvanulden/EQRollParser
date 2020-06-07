# EQRollParser
A log parser for EverQuest.

It searches for /random rolls and will print them out in order from highest to lowest roll.  The parser does need a little help to separate sets of rolls.  It looks for the output of the /who command at which point it starts reading for the next set.  So, to separate one set of rolls from the next, simple issue a /who command in your EverQuest client.

![alt text](http://www.guildseofon.com/EQRollParser/EQRollParser.gif)

## Release Notes:

1.0 Initial revision.

1.1 Now opening the log file with CFile::shareDenyNone to play nice with other log parsers (GamParse).

1.2 Changed the icon so it is easier to differentiate from the EverQuest client in the task bar.

1.3 Added search feature which allows you to search through your log for any text.

1.4 Updated solution and project to compile in Visual Studio 2019


