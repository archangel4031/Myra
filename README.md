<p align="center">
<img src="https://i.ibb.co/sX88kgy/Art-World-Logo-T.png" alt="Art World Logo" width="25%">
</p>

---


# Myra (formerly GAS Associate)

[![Discord](https://img.shields.io/discord/820665024137789472?label=discord&style=for-the-badge)](https://discord.gg/nfkTafPJKK)
[![GitHub last commit](https://img.shields.io/github/last-commit/archangel4031/Myra?color=%237d0096&style=for-the-badge)](https://github.com/archangel4031/Myra)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/archangel4031/Myra?color=%23ad0000&label=latest%20release&style=for-the-badge)](https://github.com/archangel4031/Myra/releases)
![Static Badge](https://img.shields.io/badge/Unreal_Engine-5.7-cyan?style=for-the-badge)
[![GitHub repo size](https://img.shields.io/github/repo-size/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra)
[![GitHub forks](https://img.shields.io/github/forks/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra/network/members)
[![GitHub Repo stars](https://img.shields.io/github/stars/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra/stargazers)
[![Support me on Patreon](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.vercel.app%2Fapi%3Fusername%3DArtWorldbyMalikSahab%26type%3Dpatrons&style=for-the-badge&color=black)](https://patreon.com/ArtWorldbyMalikSahab)


---
Myra is the next iteration of GAS Associate. Inspired by the Lyra Starter Project, Myra is a smaller version of Lyra (Mini-Lyra or Myra) that is easier to use. Myra tries to implement best practices from Lyra while keeping it simple and easy to use for beginners. 

**Myra plugin is compiled for Unreal Engine 5.7**

Refer to [wiki pages](https://github.com/archangel4031/Myra/wiki) on how to use the plugin in its current state. Documentation is still in progress and things may change without notice.

---

### ⚠️ WARNING ⚠️

Myra is still in alpha phase of development. It is not recommended to use Myra in production just yet. You are encouraged to contribute to the development of the new plugin and provide feedback. For older version use the plugin from the [Releases page](https://github.com/archangel4031/Myra/releases)

The default branch is now *main* instead of *master*. GAS Associate is deprecated and can be found on the *master* branch.

---

_Following is the description for old plugin version (GAS Associate)_
This will be updated once the Myra Plugin moves from alpha to stable.

### Description:

This is the source code for for the Plugin Myra, that is supposed to make your life easier while using Unreal's Gameplay Ability System.


### Information:

The plugin contains all the necessary C++ files required to make Gameplay Ability System work. This plugin allows you to jump quickly into GAS without needing to manually create each and every C++ file. I have already created tutorial series on using Gameplay Ability System with Blueprints. I highly recommend you watch the tutorial series to get familiar with Gameplay Ability System. The tutorial series also cover C++ explanations that are easy to understand even for non-programmers.

-   [UE4 Gameplay Ability System Practical Example](https://www.youtube.com/playlist?list=PLeEXbS_TaXrAbfoPYSNROqe1fDQfQHTfo)
-   [UE4 Gameplay Ability System for Blueprint Programmers](https://www.youtube.com/playlist?list=PLeEXbS_TaXrDlqQv753CpKqDlpNXixFMg)

The repo contains blank C++ based projects for Unreal 5.0
*Unreal 4.27 project is out of support and moved to [legacy](https://github.com/archangel4031/Myra/tree/legacyUE4UE5) branch*

### How to Use (for Old Version - GAS Associate):
###### Use Template Blank Project

 1. Download the Blank Project according to your required Engine Version
 2. Start working in the project directly OR
 3. Import your content in this Blank Project and start using GAS

###### Use Standalone Plugin Folder (presently for GAS Associate only)

 1. Download the Plugins folder from the [***Releases***](https://github.com/archangel4031/Myra/releases) Section according to your required Engine Version
 2. If working with a Blueprint Only Project, first add a new C++ Class of Actor. If not continue to next step
 3. Close the Editor and VS
 4. Paste the Plugins folder in the Project Root Directory
 5. Right Click on your .uproject file and select ***Generate Visual Studio Project Files***
 6. Open the Visual Studio and build the project for first time use.
 7. Once compiled, open the Engine and start using GAS

**NOTES:** 

 - See linked video for further information on the capabilities of the plugin
 - The Plugin has *Basic Error Checking* for Attribute names. Be careful while naming your attributes and only use alphabets and numbers without spaces. Name may not begin with a number. The plugin will ignore such Attribute Names.
 - Beta development is done in *[devtesting](https://github.com/archangel4031/Myra/tree/devtesting)* branch. If you are a programmer, contribute code for development and improvement of this plugin!
 
 **Known Issues**
 - **FOR Mac Users Only:** You will need to manually add `NetCore` to `PublicDependencyModuleNames` array in ***YourProject.Build.cs*** file. (Refer to [this issue](https://github.com/archangel4031/Myra/issues/1) for more information.)
 
### Video:
Refer to the video for more information. (Video made for Plugin Ver 1.0.0)
[GAS Associate | A plugin for Unreal Engine 5 and Unreal Engine 4](https://youtu.be/ett9ZTHYN8g)



#### Changelog

See [CHANGELOG file](https://github.com/archangel4031/Myra/blob/main/CHANGELOG.md)