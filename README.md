# Project Bulldog

### Overview
- Project Bulldog is an in development project that intends to be a third person arena action game.
- The developer, in an effort to create a handcrafted and authentic experience, has decided to not use an off the shelf engine such as Unreal5 or Unity, and instead roll their own tech
- The stack of choice will be C++, for hopefully obvious reasons, and Vulkan as the initial graphics API to maximize cross platform PC support.
- In an ideal world, additional proprietary graphics APIs will be added to support console platforms.

### Setup
Presently there are no prebuilt binaries, and it must be compiled from source
> - Windows setup
1. Clone repository
> - Dependencies
2. Download Vulkan SDK https://vulkan.lunarg.com/sdk/home#windows
> - Verify vulkan is installed by running ```$ vkvia``` in a terminal. You should get the spinning LunarG cube
In my opinion it's worth just downloading Visual Studio and using that toolchain. 
If you think otherwise, the build system revolves around cmake, so go nuts.
1. Download VS2022 (Community is fine) > https://visualstudio.microsoft.com/vs/
2. Run the installer
3. Select the 'Desktop development with C++' workload during setup. 
4. Finish the install, launch Visual Studio, open the repository, VS should auto detect the Cmake file and handle it from there

### Current state
- Initial functionality is going to be based off the vkguide.dev resource, as it does a good job with an initial structure, handling Vulkan 1.3's dynamic approach, and a relevant but not overly bloated 3rd party library list
- As of now this includes:
1. GLTF loading
2. Per-object transforms (not yet dynamic)
3. A basic FPS-style camera
4. Compute based background

#### Planned features
- I do not have a concrete plan on what is going to be added. 
- One of the benefits that I am hoping to get out of rolling my own tech is a relatively lightweight tech stack, and only creating what I need.
That said, here is the current list:
- GLTF Material support
- A 'better' abstraction for objects. This will be pretty open ended as physics, scripting, and editor support will stem off of this.
- More formal binding for scenes/levels. Right now the program just loads a hardcoded GLTF file path and renders a few items in that, which leaves much to be desired


#### Structure
- Presently, the project contains an 'engine', 'lib', and 'shaders' directory. Engine contains my source code, lib contains all third party libraries that are in use or planned for use, and shaders will contain shader code for rendering
- At some point, there will be a separate game files directory, for scripting, map data, and so on. I will likely keep the source of this private to some capacity, but keep an uploaded build of anything playable. I do intend to try and sell the game and having public source slightly defeats that purpose
 