# AsynchronousMachine

## Get familiar

This project introduces the concept of data objects holding arbitrary data content
and let it communicate among each other through the concept of a reactor via callbacks.

To hold data objects and callbacks together forming a higher object the concept of a
module has been introduced too.

To get in the project is divided into git branches you should go through step by step.

Start with following sequence of git branches:

1. concept-dataobject
2. concept-reactor
3. concept-reactor-threaded
4. concept-module
5. concept-reflection
6. concept-timer
 
Other git "dev/branches" are used for temporary tests!

## How to use

Install boost libraries.

Take care how to find them by your compiler.

Change your codeblocks projects settings accordingly.

## TODO

1. Test to replace std::result_of_t<Visitor(D)> get(Visitor visitor) const by auto get(Visitor visitor) const
2. Add special use cases for indentionally misuse of set/get
3. Update newest timer branch if changes has been made
