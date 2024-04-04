## Manual for building the project
### Prerequisites
* Vulkan SDK installed in the OS
    1) Windows - https://www.lunarg.com/vulkan-sdk/
    2) Raspberry PI 4/5 (Video with installation) - https://www.youtube.com/watch?v=TLzFPIoHhS8&t=78s&ab_channel=NovaspiritTech
* CMake 3.14 (at least) installed in the OS
* Development libraries are located in the `external` folder - these are downloaded with repository


### Building for Windows
* Install Visual Studio Community/Professional
* Clone the repository
* Open repository folder with Visual Studio
* Open CMakeLists.txt and save the file (ctrl + s)
* On the toolbar next to the green triangle select the target to run (VulkanProject.exe or VulkanTests.exe)

### Building for Raspberry PI
* Clone the repository
* Go to the repository with `cd` command
* `$ cmake -B build -S .`
* `$ cd build`
* `$ make`
* `$ cd bin`
* `$ ./VulkanProject`