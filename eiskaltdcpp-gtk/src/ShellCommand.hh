//Class to take care of communication with the shell
//Author: Irene

#ifndef SHELL_COMMAND_HH
#define SHELL_COMMAND_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include "settingsmanager.hh"

class ShellCommand
{
	public:
		/* input = shell command or name a script in the Script directory
		   len (optional) = maximum resultsize, standard set to 256
		   shell (optional) = 0 for a script, 1 for a shellcommand */
		ShellCommand(char* input, int len=265, bool shell=0);
		~ShellCommand();
		bool Error(); //Returns true if an error has occurred
		char* Output(); //Returns output. If unfixable error has occurred, output = ""
		char* ErrorMessage(); //Returns errormessage. If no error has occurred, errormessage = ""
		int GetResultSize(); //Returns size of the result

	private:
		char* output;
		char* errormessage;
		bool error;
		int resultsize;
};

#else
class ShellCommand;
#endif
