# Myra (formerly GAS Associate)

[![Discord](https://img.shields.io/discord/820665024137789472?label=discord&style=for-the-badge)](https://discord.gg/nfkTafPJKK)
[![GitHub last commit](https://img.shields.io/github/last-commit/archangel4031/Myra?color=%237d0096&style=for-the-badge)](https://github.com/archangel4031/Myra)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/archangel4031/Myra?color=%23ad0000&label=latest%20release&style=for-the-badge)](https://github.com/archangel4031/Myra/releases)
![Static Badge](https://img.shields.io/badge/Unreal_Engine-5.7-cyan?style=for-the-badge)
[![GitHub repo size](https://img.shields.io/github/repo-size/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra)
[![GitHub forks](https://img.shields.io/github/forks/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra/network/members)
[![GitHub Repo stars](https://img.shields.io/github/stars/archangel4031/Myra?style=for-the-badge)](https://github.com/archangel4031/Myra/stargazers)

---
Myra is the next iteration of GAS Associate. Inspired by the Lyra Starter Project, Myra is a smaller version of Lyra (Mini-Lyra or Myra) that is easier to use. Myra tries to implement best practices from Lyra while keeping it simple and easy to use for beginners. 

**Myra plugin is compiled for Unreal Engine 5.7**

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
- Ver 2.0.0-alpha
    - Project renamed to Myra.
    - Rewritten from scratch following Lyra Starter Game Best Practices.
-   Ver 1.6.1
    - Compied for UE 5.5
-   Ver 1.6.0
    -   Added support for [Advanced Locomotion System - Community Version](https://github.com/dyanikoglu/ALS-Community)
    -   Simply Download the ALS Plugin and paste in Project **Plugins** Directory
    -   Added option in GAS Associate Helper to use Plugin with or without ALS _(Experimental)_
-   Ver 1.5.1
    -   Update plugin project to Unreal Engine 5.1 The project name has also changed
    -   Update API to Unreal Engine version 5.1. Code changed in Target Actor Classes
    -   Added prompt for using GAS Associate Helper. Now the plugin will explicitly ask you to press _Yes_ before modifying the files
-   Ver 1.5.0
    -   Added functions for setting Attribute Values directly from Character Class. These can be used to initialize Attributes from Save File for example.
    -   Updated GAS Associate Helper to create Setter functions for changing Attributes directly. Now GAS Associate Helper will also create these Setter functions for each Attribute
-   Ver 1.4.1
    -   Improved error handling for Attribute Names
    -   Added error handling for empty starting data. Not setting Default Starting Data with Attribute Table will no longer crash the Engine
-   Ver 1.3.0
    -   Added error handling for Attribute Names. Now GAS Associate Helper will take proper inputs for Attribute Names. It will ignore wrong / invalid syntax names
    -   Added error handling for empty DefaultGASAttributes.ini file. In case of any error reading the file, plugin will auto add a default attribute of Health and Max Health
    -   Added separate branch for in-development version for public testing (beta version of plugin will go in _[devtesting](https://github.com/archangel4031/Myra/branches)_ branch)
-   Ver 1.3.0
    -   Added a proper Target Actor for ground trace. This Target Actor is based of built in Ground Trace Target Actor Class
    -   Removed support for UE4. The UE4 will now exist on _[legacy](https://github.com/archangel4031/Myra/tree/legacyUE4UE5)_ branch
-   Ver 1.2.2
    -   Added an option to select Replication Mode for Ability System Component in Attribute Helper Wizard. Now you can use Full mode for single player games
    -   Default Replication Mode is set to **_Full_**
-   Ver 1.2.1
    -   Added Get Ability Info Function in Character Class as well. Now you can get GA Class info in Character Class to update UI Elements
    -   Fixed spelling mistake in GAS Character Class
    -   Fixed Initial Data Table with values for all example Attributes
-   Ver 1.2.0
    -   Added Get Ability Info Function in GAS Gameplay Ability. Now you can retrieve information about Ability Cost and Cooldown using this function. (Currently only Modifier based Cost is supported)
    -   Added a separate [**Release**](https://github.com/archangel4031/Myra/releases) for Ver 1.2.0
-   Ver 1.1.0
    -   Added an option to automatically create a Max Value Attribute to handle clamping (see Example Project)
    -   Added Server Only functions that only execute as server. Generic versions of functions still exists and may be useful in some cases
    -   More comments in C++ Code
