# Project Bulldog

### Overview
- Project Bulldog is an in development game that intends to be a third person arena game.

- The developer, in an effort to create a handcrafted and authentic experience, has decided to not use an off the shelf engine such as Unreal5 or Unity, and instead roll their own tech
- The stack of choice will be C++, for hopefully obvious reasons, and Vulkan as the initial graphics API. 
- In an ideal world, additional proprietary graphics APIs will be added to support console platforms


### Current state
- Most of the project at current writing follows online resources, as there is lots of learning ahead.
- I have a pretty solid base from initial learning of Vulkan and a Computer Science bachelors, however industry standards on things like engine design, build processes, and even a C/C++ standard are lacking
- For those reasons, I am choosing to follow the vkguide.dev style, as some standard is better than none. 

#### Planned features
- I do not have a concrete plan on what is going to be added. 
- One of the benefits that I am hoping to get out of rolling my own tech is a relatively lightweight tech stack, and only creating what I need.


#### Structure
- Presently, the project contains an 'engine', 'lib', and 'shaders' directory. Engine contains my source code, lib contains all third party libraries that are in use or planned for use, and shaders will contain shader code for rendering
- At some point, there will be a separate game files directory, for scripting, map data, and so on. I will likely keep the source of this private to some capacity, but keep an uploaded build of anything playable. I do intend to try and sell the game and having public source slightly defeats that purpose
 
