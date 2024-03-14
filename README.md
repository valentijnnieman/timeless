# Timeless Engine

This is my 2d game engine in C++ that I use to build games. It uses an ECS architecture (entity component system) and is very much a work in progress!

## Header-only
To ease with development, and since the classes are relatively compact, the source code is in header files only at this point. I found that this resembles how source code is organized in more modern languages - which is where I'm coming from - which is why I chose this for now. As the project grows, more platforms are supported, and when it is getting ready for an actual 1.0 release, this will probably change!

## Time sequencing
This engine is currently being used to develop a game based on a 2d (isometric!) grid, where objects act on a fixed time scale. Think of it like a music sequencer, where along fixed time intervals notes are fired, creating music. 
The idea of this engine is that users can create a grid (defined in `algorithms/graph.hpp`) and create `Behaviour`s for objects placed on that grid. A `Behaviour` is really just a vector of lambda functions and some other bits, which the `NpcAiSystem` (name will likely change) takes and sequences. The system will take all the behaviours, and fire the next behaviour for all registered entities at a user defined time interval. It can also, instead of the next behaviour, get the previous one, which is useful as a "go back in time" mechanic or an undo mechanic etc.

## Should I use this?
At the moment, it probably does not make a ton of sense to use this early-stage game engine, unless you really are intrigued by the graph + time sequencing features and don't mind breaking changes, or if you want to help develop this!

## Thank you
Thanks for checking this out!
