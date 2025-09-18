# NFA 2 Regex

A C++ Program that converts NFA / DFA into Regex

Asks simple questions to describe an NFA:

- How many states?
- How many accepting states?
- Transition functions?


Follows the process of removing one state at a time, converting the NFA into a more compacted GNFA, which eventually shrinks down to two states, and one transition, which happens to be the regex. 
