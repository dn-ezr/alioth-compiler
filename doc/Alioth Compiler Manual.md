---
title: Alioth Compiler Manual
author: GodGnidoc
date: 2019/07/17
---

# 1. About this manual

This manual is written for users of the compiler of the Alioth programming language. The corresponding compiler version is `V0.3`. This manual states all functions provided by the compiler and how to use them. This document is written in UTF8 encoding, formatted in markdown format.

> Note:  
> &nbsp;&nbsp;&nbsp;&nbsp;The old version of this compiler was named "**aliothc**", the last letter 'c' means "**compiler**". But compiling is no more the only function of this program for now, so the program is renamed as "**alioth**", please type the correct command according the version of compiler you're using.

- [1. About this manual](#1-About-this-manual)
- [2. Basic concepts](#2-Basic-concepts)
  - [2.1. Target](#21-Target)
    - [2.1.1. Auto](#211-Auto)
    - [2.1.2. Executable](#212-Executable)
    - [2.1.3. Static](#213-Static)
    - [2.1.4. Dynamic](#214-Dynamic)
    - [2.1.5. Validate](#215-Validate)
    - [2.1.6. Repository](#216-Repository)
    - [2.1.7. Package](#217-Package)
  - [2.2. Space](#22-Space)
    - [2.2.1. Main space](#221-Main-space)
    - [2.2.2. Sub space](#222-Sub-space)
    - [2.2.3. Extra space](#223-Extra-space)
  - [2.3. Package](#23-Package)
    - [2.3.1. Sections](#231-Sections)
    - [2.3.2. main section](#232-main-section)
    - [2.3.3. dev section](#233-dev-section)
    - [2.3.4. other section](#234-other-section)
    - [2.3.5. Locating](#235-Locating)
  - [2.4. Repository](#24-Repository)
  - [2.4.1. Static repository](#241-Static-repository)
  - [2.4.2. Remote repository](#242-Remote-repository)
- [3. Compiling targets](#3-Compiling-targets)
  - [3.1. Auto target](#31-Auto-target)
  - [3.2. Excutable target](#32-Excutable-target)
  - [3.3. Static target](#33-Static-target)
  - [3.4. Dynamic target](#34-Dynamic-target)
  - [3.5. Validate target](#35-Validate-target)
- [4. Managing targets](#4-Managing-targets)
  - [4.1. Package managing](#41-Package-managing)
    - [4.1.1. Pack](#411-Pack)
    - [4.1.2 Install](#412-Install)
    - [4.1.3. Update](#413-Update)
    - [4.1.4 Remove](#414-Remove)
- [Appendix A: Table of command line options](#Appendix-A-Table-of-command-line-options)
  - [Target indicators](#Target-indicators)

# 2. Basic concepts

There are some basic concepts you need to know before you can properly use the compiler.

Some of them has the same name as the other concept but carries different meaning, you can add prefixes to solve this problem within certain context if necessary, like "Alioth Repository", "Alioth Target" etc.

## 2.1. Target

Unlike the common compilers always process some source documents as input and generate a file in certain format as output, the compiler of the Alioth programming language does more.

Basically you can just execute command to run the compiler to generate some output file from source documents. Furthermore you can start the compiler to host a repository or to host a package.

A **target** is an object which is the reason why you run the compiler. May be the output file in format of static link library or dynamic link library vs executable entity. May be the hosted repository or the hosted package.

### 2.1.1. Auto

Compiler will try to generate static link library or binary executable entity. It depends on wether compiler could find the entry mark within source code.

### 2.1.2. Executable

Compiler will try to generate binary executable entity from source code.

### 2.1.3. Static

Compiler will try to generate static link library, regardless whether there was an entry mark.

### 2.1.4. Dynamic

Compiler will try to generate dynamic link library.

### 2.1.5. Validate

Compiler will validate semantics within the work space, and output the diagnostics informations.

### 2.1.6. Repository

Compiler will start the host mode and stay running until you interrupt the execution. Compiler will accept and process requests from other remote compilers to provide compiling resources hosted in this repository.

### 2.1.7. Package

Compiler will send package informations to the specified repository host. Then, depending on the configuration, compiler will choose to exit or stay running to process resource requests.

## 2.2. Space

The abstract location where compiler will look for compiling resources in. Generally the compiler maps **space**s to file system path, you could change that by command line parameters or configure files.

### 2.2.1. Main space

Main spaces have no parent space, they are used to contain sub spaces or files.

- root
  
  The root space is used to contain configuration files and standard libraries for the alioth programming language; There is only one root space each time the compiler runs.
- work

  The work space is used to contain compiling resources would be used for the current target; There is only one work space each time the compiler runs.
- apkg

  The spaces of apkg kind are used to host packages for the Alioth programming language software development kits. SDKs installed by alioth compiler will be managed in certain path, therefor their home dir can be scaned to be apkg space.

### 2.2.2. Sub space

Sub spaces are those spaces who need their parent spaces to make them locatable. There are several subs spaces defined, each of them has the exact duty.

- arc

  Sub spaces used to contain static link libraries produced by the compiling target.
- bin

  Sub spaces used to contain executable files prodced by the compiling target.
- doc

  Sub spaces used to contain documents generated by compiler.
- inc

  Sub spaces used to contain source code documents written by user.
- lib

  Sub spaces used to contain dynamic link libraries produced by the compiling target.
- obj

  Sub spaces used to contain object files generated by compiler.
- src

  Sub spaces used to contain source code documents written by user.

### 2.2.3. Extra space

Spaces listed above are the standard abstract spaces, there maybe other spaces existing but not to be the standard abstract spaces. There is a space type named extra space which is used to represent these spaces.

## 2.3. Package

Package is the biggest unit when to install, remove or refer. You can modify the config file to decide the resources you want to share.

When installing a package using the compiler of the Alioth programming language, you can choose which section you want, then compiler will download and install those resources involved by the sections you have chosen.

Compiler will ensure every packages released has at least two main sections, which are the section "main" and the section "dev".

### 2.3.1. Sections

Every sections has a attribute set describes dependencies of this section and other information about this section.

### 2.3.2. main section

No matter the package is a library or an application, main section should always provide the final product.

The major difference is whether have you to build an environment to develop the package. The answer is no, if you're installing the "main" section.

### 2.3.3. dev section

The "dev" section always carries source code of this package, which is used to develop this package. This kind of section could be used when cooperating with others to develop this package or you have to modify something in this package.

### 2.3.4. other section

You can design a section to host document resources or something else. The most difference between all sections is the list of resources to be shared.

### 2.3.5. Locating

There are two situation where you have to express the location of a package, one of them is when you're installing or removing a package within the command line, another situation is that when you're coding the dependency descriptor.

Once a package is installed locally, just a package name is enough to locate a package.

Packages in remote repositories or file system can be referred by uri, unless you forbid it, compiler will install those packages automatically.

Generally the package name can be any non-empty string, but colon can not be one character of this string, because it is used to indicate the version of the package when locating packages.

Example:

~~~alioth
module Hello :
  network @ 'stdlib-linux-x86_64:1.1.47' as this module
~~~

Any dependency indicates the **main** section of the package.

## 2.4. Repository

Repository is the container of packages. 

## 2.4.1. Static repository

There always a static repository which is used to host packages installed locally, there is no need to run a compiler to host it.

## 2.4.2. Remote repository

You can host your own repository on your server. Remote repositories can be located via `URI`, scheme is `alioth://`.

The scheme `alioth` stands for the `alioth compiler communicating protocol`, the format of the uri as follow:

~~~
alioth://[<user>@]<host>/<repository>[?<options>]
~~~

# 3. Compiling targets

The basic function of this compiler is to compile compiling resources such as source code documents or libraries into the target output file formated in the specified format.

## 3.1. Auto target

Defautly, compiler runs the `Auto target`. The only argument required by the `Auto target` is the name of this target.

Command format as follow:

~~~bash
#!/bin/bash

alioth : Hello
~~~

There are three tokens in the command listed above.

Token "alioth" is the program name of this compiler.

The colon is called the **target indicator**, which is used to indicate an **auto target**. A target name right following it is required.

The third token "Hello" is the target name of this target, it is supposed to be a normal non-empty string.

Target indicator and the target name, they are **pre argument**s.

Compiler will scan all modules available in the work space and compile them into the target. You can give a list of module name to make compiler compiles modules specified only. The order of the list doesn't matter.

~~~bash
#!/bin/bash

alioth First : Hello Second Third
~~~

The command shown above indicates the compiler will compile three modules which are named "First", "Second" and "Third" into a target named "Hello".

## 3.2. Excutable target

Change the target indicator from a single colon to the following format, compiler will consider this target as an executable target, and try to generate executable entity from source code. If there's no entry mark can be found, compiler reports an error.

~~~bash
#!/bin/bash

alioth x: Hello
~~~

Since no module name can end up with a colon, there is no need to worry about command conflicts.

As you maybe already imagine, all target indicators end up with colon, by changing the prefix of the indicator, you can specify different target.

## 3.3. Static target

Static target try to generate a static link library. The output file will be named as `lib<TARGET-NAME>.a` and be placed in the arc sub space.

~~~bash
#!/bin/bash

alioth s: Hello
~~~

A file named `provide.json` will be modified, or created if it isn't existing.

The file `provide.json` describes a map between modules and libraries, these information will be extracted and used when building a package involving libraries.

File `provide.json` is placed in the doc sub space.

## 3.4. Dynamic target

Dynamic target try to generate a dynamic link library. The output file will be named as `lib<TARGET-NAME>.so` and be placed in the lib sub space.

~~~bash
#!/bin/bash

alioth d: Hello
~~~

The file `provide.json` will be modified also.

## 3.5. Validate target

Compiler will try to validate semantics within the workspace, all diagnostics informations will be formatted and output via the standard output stream.

~~~bash
#!/bin/bash

alioth v: check
~~~

The target name is needed since the format is defined.

# 4. Managing targets

The other kind of function of this compiler is to manage resources.

## 4.1. Package managing

Compiler of the Alioth programming can be used to manage packages.

### 4.1.1. Pack

To pack a package from the work space, use the indicator `package: <PACKAGE-NAME> <SECTIONS>`.

~~~bash
#!/bin/bash

alioth package: stdlib-linux-x86_64 main
~~~

A package file named in format `<PACKAGE>.<SECTION>.apkg` will be generated and stored into the work space.

You can edit the config file named `packages.json` in the doc sub space to specify some options for generating packages. Generating will abort if there is no corresponding section in the config file.

~~~json
{ 
  /** file packages.json defines attributes of all packages
      each package is a sub object of the root object of this file.
      The object name is the package name.*/
  "stdlib-linux-x86_64" : {
    "version" : "1.3.47", /** The version number is formatted as `major.minor.patch`.*/
    "sections" : {
      "main" : {
        "resources" : [
          "lib/*.so",
          "arc/*.a",
          "inc/*.alioth"
        ],
        "dependencies" : [/** If any dependency of this package is another alioth package, describe it here, alioth will help you to manage it. */],
        "scripts" : {
          "pre-install" : "",
          "post-install" : "",
          "pre-remove" : "",
          "post-remove" : "",
          "pre-update" : "",
        }
      }, "dev" : {
        "resources" : [
          "inc/*.alioth",
          "src/*",
          "makefile"
        ]
      }
    }
  }
}
~~~

As we have talked about, auto-generated file `provide.json` will be read and necessary contents in it will be packed.

You may pack more than one sections into one package file :

~~~bash
#!/bin/bash

alioth package: stdlib-linux-x86_64 main doc
~~~

### 4.1.2 Install

No matter how did you get the package file, an directory named with the package name will be created when installing the package; That directory is used to store all sections of this package.

Use the indicator `install: <PACKAGE> - <SECTIONS>` to install package.

~~~bash
#!/bin/bash

alioth install: alioth://opengl.org/OpenGL/opengl-linux-x86_64 - main
~~~

A file named `package.json` will be generated from the package informations, which is used to manage the resources in this package. The format of this file is very similer as the format of the file `packages.json`.

Take the version number after the package name, combine them using a colon, then you can specify the version you want to use.

~~~bash
#!/bin/bash

alioth install: alioth://opengl.org/OpenGL/opengl-linux-x86_64:3.7.^ - main
~~~

The up arrow in the version number means pick the highest possible number at this position.

### 4.1.3. Update

There is only one version of packages can be installed at once, if the **section** you want to install come from the package who has a higher version, you must update the package installed to the corresponding version.

Use the indicator `update: <PACKAGE> [- <VERSION>]` to update a package.

~~~bash
#!/bin/bash

alioth update: stdlib - 3.^
~~~

- If no target version gaven, compiler try to update the package to the highest version.
- If the package was install via remote repository, the host has been recorded, there will be no need for you to specify the remote repository address again.
- All sections must be able to be upgraded once for good, or update progress will fail.

### 4.1.4 Remove

It's easy to remove a section or package, just use the indicator `remove: <PACKAGE> [- <SECTIONS>]` to do it.

~~~bash
#!/bin/bash

alioth remove: stdlib-linux-x86_64 - main dev doc
~~~

The main section could be dependency of other packages, if it happens compiler will ask if you really want to remove that section.

# Appendix A: Table of command line options

## Target indicators

|option|format|instance|comment|
|:--|:--|:--|---|
|`:`|`: <TARGET-NAME>`|`: Hello`|Auto target indicator|
|`x:`|`x: <TARGET-NAME>`|`x: Hello`|Executable target indicator|
|`s:`|`s: <TARGET-NAME>`|`s: Hello`|Static link library target indicator|
|`d:`|`d: <TARGET-NAME>`|`d: Hello`|Dynamic link library target indicator|
|`v:`|`v: <TARGET-NAME>`|`v: Validate`|Validate target indicator|
|`package:`|`package: <PACKAGE-NAME> <SECTION-NAME>`|`package: OpenGL main`|Package target indicator|
|`install:`|`install: <PACKAGE> - <SECTIONS>`|`install: pack.apkg - main doc`|Section install target indicator|
|`update:`|`update: <PACKAGE> [- <VERSION>]`|`update: stdlib - 3.7.^`|Package update target indicator|