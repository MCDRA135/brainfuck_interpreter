Brainf\*ck interpreter made using Python 3.9.4. Includes an interactive mode that resembles Python's shell, because who never dreamt of having an interactive shell for testing their Brainf\*ck code ?

## Usage
- `python brainfuck.py [file]` : Execute Brainf\*ck code from a file.
- `python brainfuck.py`        : Launch interactive Brainf\*ck shell.

## Interactive shell
Note that the interactive shell does NOT reset the memory state after each input.
Interactive shell includes special commands, which cannot be used combined with Brainf*ck characters :
- $ : Debug command, prints the memory array.
- @ : Reset memory state
- ! : Exit the shell.

# Now in C version for faster Brainf\*ck execution
Was made using only standard C, so should be platform- and compiler-independant.

## Usage
- `./brainfuck [file]`        : Execute Brainf\*ck code from a file.
- `./brainfuck`               : Launch interactive Brainf\*ck shell.

## Interactive shell
Interactive shell special commands are the same as Python version, but it's faster ! 
