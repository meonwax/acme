

      --- UltraEdit wordfile for ACME assembler source codes ---



The word list is freeware - anyone may use it as they see fit,
commercially or otherwise. My only restriction is that I won't be held
responsible for any damages caused by using it.

Instructions to use the wordlist:

The contents of the wordfile should be pasted in at the end of the
file called "Wordfile.txt" in the UltraEdit program directory, and
then you should check that the number on the first line of the
assembler language definition doesn't conflict with any other language
in the wordfile. It currently reads "/L10 ..." but instead of 10 you
could put in any number between 1 and 20. 1-9 are normally already
taken, so 10 is a good choice if this is the only language you've
added to UltraEdit.
Restart UltraEdit if it was running. That's it.

There's more information in UltraEdit's built-in
help and on this page:
http://www.ultraedit.com/index.php?name=Content&pa=showpage&pid=40#wordfiles

The only thing which I think may seem odd to some people is that some
of the opcodes are marked red. That's the instructions that may change
the value of the program counter (JMP, JSR, RTS, BEQ etc). I think it
makes it easier to follow the program flow when reading the code.
Anyway, this can be changed so that they have the same color as the
other opcodes. It's in the "Advanced | Configuration" dialog box in
UltraEdit.

Best regards,

Fredrik Ramsberg
