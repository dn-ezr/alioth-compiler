---
title: Alioth Compiler Manual
author: GodGnidoc
date: 2019/07/17
---

# 1. About this manual

This manual is written for users of the compiler of the Alioth programming language. The corresponding compiler version is `V0.3`. This manual states all functions provided by the compiler and how to use them. This document is written in UTF8 encoding, formatted in markdown format.

> Note:
> &nbsp;&nbsp;&nbsp;&nbsp;The old version of this compiler was named "**aliothc**", the last letter 'c' means "**compiler**". But compiling is no more the only function of this program for now, so the program is renamed as "**alioth**", please type the correct command according the version of compiler you're using.

- [1. About this manual](#1-about-this-manual)
- [2. Basic concepts](#2-basic-concepts)
  - [2.1. Target](#21-target)
    - [2.1.1. Auto](#211-auto)
    - [2.1.2. Executable](#212-executable)
    - [2.1.3. Static](#213-static)
    - [2.1.4. Dynamic](#214-dynamic)
    - [2.1.5. Validate](#215-validate)
    - [2.1.6. Repository](#216-repository)
    - [2.1.7. Package](#217-package)
  - [2.2. Space](#22-space)
    - [2.2.1. Main space](#221-main-space)
    - [2.2.2. Sub space](#222-sub-space)
    - [2.2.3. Extra space](#223-extra-space)
  - [2.3. Package](#23-package)
    - [2.3.1. Sections](#231-sections)
    - [2.3.2. main section](#232-main-section)
    - [2.3.3. dev section](#233-dev-section)
    - [2.3.4. doc section](#234-doc-section)
    - [2.3.5. Locating](#235-locating)
  - [2.4. Repository](#24-repository)
    - [2.4.1. Static repository](#241-static-repository)
    - [2.4.2. Remote repository](#242-remote-repository)
- [3. Compiling targets](#3-compiling-targets)
  - [3.1. Auto target](#31-auto-target)
  - [3.2. Excutable target](#32-excutable-target)
  - [3.3. Static target](#33-static-target)
  - [3.4. Dynamic target](#34-dynamic-target)
  - [3.5. Validate target](#35-validate-target)
    - [3.5.1. Interactive mode](#351-interactive-mode)
    - [3.5.2. Full-interactive mode](#352-full-interactive-mode)
- [4. Managing targets](#4-managing-targets)
  - [4.1. Package managing](#41-package-managing)
    - [4.1.1. Pack](#411-pack)
    - [4.1.2 Install](#412-install)
    - [4.1.3. Update](#413-update)
    - [4.1.4 Remove](#414-remove)
    - [4.1.5. Publish](#415-publish)
  - [4.2. Repository managing](#42-repository-managing)
- [5. Other targets](#5-other-targets)
  - [5.1. Help target](#51-help-target)
  - [5.2. Version target](#52-version-target)
  - [5.3. Init target](#53-init-target)
- [Appendix A: Table of command line options](#appendix-a-table-of-command-line-options)
  - [Target indicators](#target-indicators)
  - [Options](#options)
- [Appendix B: Configurations and config files](#appendix-b-configurations-and-config-files)
  - [Diagnostic](#diagnostic)
  - [Packages](#packages)
    - [Rules about package name](#rules-about-package-name)
    - [Packages.package](#packagespackage)
  - [Package](#package)
  - [Provide](#provide)
  - [Repository](#repository)
  - [Space](#space)
  - [Targets](#targets)

# 2. Basic concepts

There are some basic concepts you need to know before you can properly use the compiler.

Some of them has the same name as the other concept but carries different meaning, you can add prefixes to solve this problem within certain context if necessary, like "Alioth Repository", "Alioth Target" etc.

## 2.1. Target

Unlike the common compilers always process some source documents as input and generate a file in certain format as output, the compiler of the Alioth programming language does more.

Basically you can just execute command to run the compiler to generate some output file from source documents. Furthermore you can start the compiler to host a repository or to host a package.

A **target** is an object which is the reason why you run the compiler. May be the output file in format of static link library or dynamic link library or executable entity. May be the hosted repository or the hosted package.

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

When installing a package using the compiler of the Alioth programming language, you can choose which section you want, then compiler will download and install those resources involved by the sections you have chosen. In most situations the compiler automatically choose sections to download and install according to the perpose.

Compiler will ensure every packages released has at least two main sections, which are the section "main" and the section "dev".

### 2.3.1. Sections

Every sections has a attribute set describes dependencies of this section and other information about this section.

There are four available sections defined which are section `dev`, `main`, `doc` and section `src`.

### 2.3.2. main section

No matter the package is a library or an application, main section should always provide the final product.

The major difference is whether have you to build an environment to develop the package. The answer is no, if you're installing the "main" section.

When packing, only resources in those three subdirs are accepted, which are subdir `BIN`,`ARC` and `LIB` . Compiler will generate `providing table` to describe what modules are provided by this section, and which target file contains it. There must be up to only one target file become the provider of certain module.

Resorces in extra space are accepted too.

### 2.3.3. dev section

The "dev" section always carries source code of this package, only source code in the subdir `INC` are accepted. This section is installed when this package is the developing dependency of a developing package.

### 2.3.4. doc section

The doc section contains resources located in subdir `DOC` only, and no alioth configuration file can be accepted.

### 2.3.5. Locating

There are two situation where you have to express the location of a package, one of them is when you're installing or removing a package within the command line, another situation is that when you're coding the dependency descriptor.

A package name is a string without special punctuations like `.` or `-` . A string consists of all informations needed is called package id, which is formated as follow:

~~~
publisher.name-platform-arch:version
~~~

For example:

~~~
org.alioth.stdlib-linux-x64:3.0.1
~~~

Only field `publisher` and field `name` are necessary every time, other field will be filled according to configurations automatically.

Any dependency indicates the **main** section of the package.

## 2.4. Repository

Repository is the container of packages.

### 2.4.1. Static repository

There always a static repository which is used to host packages installed locally, there is no need to run a compiler to host it.

### 2.4.2. Remote repository

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

alioth v: 1
~~~

The target name specifies where to display or storge the diagnostics informations.

There're two kind of format available for this target name.

- number : To specify an output stream file descriptor.
- path/uri : To specify an document to storge the diagnostics informations.

### 3.5.1. Interactive mode

Generally, this target is started by IDE to provide the dynamic semantic diagnostic service. Sometimes, some of the source code documents are not saved to disk yet, compiler cannot read the lastest version of source code from disk, so that it may be gives diagnostics informations useless.

Use the `interactive` mode to solve this problem.

~~~bash
#!/bin/bash

alioth v: check -- 0/1
~~~

The command shown above starts the compiler in the interactive mode, the option `-- 0/1` means use the standard input and output streams to communicate with IDE.

In the `interactive` mode, compiler will send a asking package to the output stream to ask whether the IDE have the resource the compiler needs.

like this: `{"cmd":"requestContent","url":"./src/hello.alioth"}` and if the IDE do have contents of the document the compiler asked, it should respond those contents to the compiler: `{"cmd":"respondContent","status":"success","data":"...."}`.

Otherwise, if the IDE do not have the content of the resource which the compiler asked, it returns a failure package to the compiler, and then, compiler should fallback to the normal method to access this resource it needs, for example, reading from the filesystem.

### 3.5.2. Full-interactive mode

To provide in-time diagnostic service, the diagnostic process has to be triggered immediately every time the source code changed. It will be a large cost if the compiler start over the whole diagnostic progress every time. Use the `full interactive mode` to make the compiler cache the syntax trees constructed, and stand by once a diagnostic cycle ended.

The option `--- 0/1` means use the standard input and output streams to communicate with IDE. The compiler will read all signatures in work space once it is started, and then stand by.

Commands like `updateDocument`, `removeDocument`, `setWorkSpace` can be received from the standard input stream, these commands can be used by IDE to control the compiler.

# 4. Managing targets

The other kind of function of this compiler is to manage resources.

> Note that not all functions about managing opened, consider these features are coming soon in the future version.

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
    "license" : "doc/license",
    "sections" : {
      "main" : {
        "resources" : [
          "lib/*.so",
          "arc/*.a",
          "inc/*.alioth"
        ],
        "dependencies" : [/** If any dependency of this package is another alioth package, describe it here, alioth will help you to manage it. */],
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

Nodejs defined its concepts about the file `package.json` of course, we are different.

The packages of Alioth programming language are mainly play the role of libraries, as for applications we believe that developers prefer their own way to publish their products.

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

Give the command without sections specified, compiler will remove the whole package.

~~~bash
#!/bin/bash

alioth remove: stdlib-linux-x86_64
~~~

### 4.1.5. Publish

You can publish your packages to remote repositories, so that people can find and install your packages easier.

There're two mode to publish your packages.

- online mode

  Compiler send meta informations only, and then setup a host ready to process requests. When the package is requested, remote repository forwards the request to the compiler host. Data connection build up between compiler host and the resource client to transfer package.
- offline mode

  Compiler send the entire package to the remote repository, let the remote repository dispatch it.

Before you can publish your packages to the remote repository, you need an account with the right privileges. We're going to talk about this in the next section.

Now, let see the command to publish your package.

~~~bash
#!/bin/bash

alioth publish: stdlib-linux-x86_64 main - alioth://localhost/Alioth
~~~

The command above publish your package to the remote repository. Compiler will choose the mode of publishing.

~~~bash
#!/bin/bash

alioth publish: stdlib-linux-x86_64 main - alioth://cdn.us.alioth.org/Alioth --offline
~~~

The command above publish your package to the remote repository, force to use the offline mode.

The corresponding option to force to use the online mode is `--online`.

The command format using the indicator `publish:` as follow:

~~~
publish: <PACKAGE-NAME> <SECTIONS> - <REPOSITORY> [--online|--offline]
~~~

There is only one daemon can be started to host packages published, commands executed after that do not start a new process, they add the job to the daemon.

## 4.2. Repository managing

> [TODO]

# 5. Other targets

Other targets provedes function like printing the version information, printing the help page etc.

These targets are also exist in other command line tools, so we keep their indicators look normal.

## 5.1. Help target

To print the help page, use the indicator `--help`, compiler will discard any other targets and print the help page.

~~~bash
#!/bin/bash

alioth --help
~~~

## 5.2. Version target

To print the version informations, use the indicator `--version`, compiler will discard any other targets and print the version informations.

~~~bash
#!/bin/bash

alioth --version
~~~

## 5.3. Init target

To initialize a project structure in a directory, use the indicator `--init <PACKAGE>`, compiler will take one variable as the package name to initialize the project structure.

~~~bash
#!/bin/bash

alioth --init "Hello World"
~~~

A directory named `Hello World` will be created, and config files such as `packages.json`, `makefile` will be generated.

# Appendix A: Table of command line options

## Target indicators

| option      | format                                              | instance                                                      | comment                                                 |
| :---------- | :-------------------------------------------------- | :------------------------------------------------------------ | ------------------------------------------------------- |
| `:`         | `: <TARGET-NAME>`                                   | `: Hello`                                                     | Auto target indicator                                   |
| `x:`        | `x: <TARGET-NAME>`                                  | `x: Hello`                                                    | Executable target indicator                             |
| `s:`        | `s: <TARGET-NAME>`                                  | `s: Hello`                                                    | Static link library target indicator                    |
| `d:`        | `d: <TARGET-NAME>`                                  | `d: Hello`                                                    | Dynamic link library target indicator                   |
| `v:`        | `v: <TARGET-NAME>`                                  | `v: Validate`                                                 | Validate target indicator                               |
| `package:`  | `package: <PACKAGE-NAME> <SECTIONS>`                | `package: OpenGL main`                                        | Package target indicator                                |
| `install:`  | `install: <PACKAGE> - <SECTIONS>`                   | `install: pack.apkg - main doc`                               | Section install target indicator                        |
| `update:`   | `update: <PACKAGE> [- <VERSION>]`                   | `update: stdlib - 3.7.^`                                      | Package update target indicator                         |
| `remove:`   | `remove: <PACKAGE> [- <SECTION>]`                   | `remove: stdlib-linux-x86_64 - main dev doc`                  | Remove sections from packages installed                 |
| `publish:`  | `publish: <PACKAGE-NAME> <SECTIONS> - <REPOSITORY>` | `publish: stdlib-linux-x86_64 main alioth://localhost/Alioth` | Publish package to remote repository                    |
| `--help`    | `--help`                                            | `--help`                                                      | Print the help page and exit                            |
| `--version` | `--version`                                         | `--version`                                                   | Print the version information and exit                  |
| `--init`    | `--init <PACKAGE>`                                  | `--init HelloWorld`                                           | Initialize a project structure for package `HelloWorld` |

## Options

| option                | format                          | instance                    | comment                                                                 |
| :-------------------- | :------------------------------ | :-------------------------- | :---------------------------------------------------------------------- |
| `--`                  | `-- <I/O>`                      | `-- 0/1`                    | Open the interactive mode, specify streams to do the I/O operation      |
| `---`                 | `--- <I/O>`                     | `--- 0/1`                   | Open the full-interactive mode, specify streams to do the I/O operation |
| `--work`              | `--work <PATH>`                 | `--work ./demo/`            | Set the path of the workspace                                           |
| `--root`              | `--root <PATH>`                 | `--root /usr/lib/alioth`    | Set the path of the root space                                          |
| `--diagnostic-format` | `--diagnostic-format <format>`  | `--diagnostic-format %i`    | Config the format of diagnostics informations                           |
| `--diagnostic-lang`   | `--diagnostic-lang <language>`  | `--diagnostic-lang chinese` | Choose the language the diagnostics informations are written            |
| `--diagnostic-method` | `--diagnostic-method <method>`  | `--diagnostic-method json`  | Choose the method to display the diagnostics informations               |
| `--diagnostic-to`     | `--diagnostic-to <destination>` | `--diagnostic-to 4`         | Choose the destination where to print diagnostics informations to       |

# Appendix B: Configurations and config files

## Diagnostic

Diagnostics informations are given formatted, you may modify the config file `Diagnostic.json` to customize the format the diagnostics informations will be given in, even the language.

~~~json
{
  "format" : "%p:%l:%c:(%E) %i -- %s",
  "languages" : {
    "chinese" : {
      "templates" : {
        "1" : {
          "sev" : 1,
          "beg" : "b0",
          "end" : "e2",
          "msg" : "方法'%R1'没有终结指令"
        }
      }, "severities" : [
        "错误",
        "警告",
        "信息",
        "提示"
      ]
    }
  }
}
~~~

There are two major sections in this config file, the first one is that named "format".

It is simply a string describes how do you want the compiler to organize the informations about the diagnostics.

Tokens starting with '%' are place-holders, stand for variables, here is a table of usable variables:

| token | variable                                      |
| ----- | --------------------------------------------- |
| `%c`  | column number                                 |
| `%C`  | column number at the end of the code involved |
| `%d`  | date formated as `YYYY/MM/DD`                 |
| `%E`  | error code                                    |
| `%i`  | diagnostics informaton                        |
| `%l`  | line number                                   |
| `%L`  | line number at the end of the code involved   |
| `%p`  | prefix information, generally, it is a path.  |
| `%s`  | severity in string                            |
| `%S`  | severity in number                            |
| `%t`  | time formated as `hh:mm:ss`                   |
| `%T`  | timestamp in number                           |

The second major section is the section named 'languages', which is a object, every key is a name of one language, and every corresponding object is a set of format descriptor, each of these descriptors describes format of a corresponding diagnostic information of certain error code.

~~~json
"1" : {
  "sev" : 1,
  "beg" : "b0",
  "end" : "e2",
  "msg" : "方法'%R1'没有终结指令"
}
~~~

The key of one descriptor is the error code written in string.

All attributes inside this descriptor are necessary, descriptions as follow:

| attribute | value                         | meaning                                                                                                                                   | comment                                                                                                                                                                                                                                                                              |
| --------- | ----------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `sev`     | `<INT>: 1 ~ 4`                | severity                                                                                                                                  | 1:Error<br/>2:Warning<br/>3:Information<br/>4:Hint                                                                                                                                                                                                                                   |
| `beg`     | `n`<br/>`b<INT>`<br/>`e<INT>` | The beginning position of source code involved by this diagnostic information                                                             | `n` means there's no beginning position<br/>As for other two format, they use a boundary of one token as the position. The starting letter specifies whether the beginning or the ending of the token will be used. The number right following the letter is the index of the token. |
| `end`     | `n`<br/>`b<INT>`<br/>`e<INT>` | The ending position of source code involved by this diagnostic information                                                                | This attribute takes the same method to express the position as the attribute `beg`.                                                                                                                                                                                                 |
| `msg`     | `<string>`                    | This string is a string with place-holders in it, tells the compiler how to organize the involved tokens to a human readable information. | This syntax will be talked about soon enough after this table.                                                                                                                                                                                                                       |

The attribute `msg` is a formatted string, describes hwo the way to organize `tokens` involved to this diagnostics information. Compiler will grab certain number of tokens according to the error code, and you can refer to them using a index, for example `%2` means the third token involved, `%1` means the second, and so on; You may add a color hint to specify the color of one token when is will be printed, for example `%R0` means print the first token involved, in a red color if possible.

Here's a table of color hint available in the formatted string.

| hint | color             |
| ---- | ----------------- |
| `r`  | red               |
| `R`  | red & **blob**    |
| `g`  | green             |
| `G`  | green & **blob**  |
| `b`  | blue              |
| `B`  | blue & **blob**   |
| `y`  | yellow            |
| `Y`  | yellow & **blob** |
| `p`  | purple            |
| `P`  | purple & **blob** |
| `c`  | cyan              |
| `C`  | cyan & **blob**   |

## Packages

In workspace, you may pack resources into packages. Use the config file `packages.json` to specify options generating packages.

The config file `packages.json` could contain configurations for more than one package, that means you can pack more than one package from your workspace.

Attributes for packages can be written in the packages scope, all packages will inherit them.

| Field              | type                                       | description                                                                                                                                                                                                                                                               | Example                                                            |
| ------------------ | ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| `authors`          | `[string...]`                              | list of authors, describe name and email formated like this:<br/>`name <email>`                                                                                                                                                                                           | `["GodGnidoc <godgnidoc@outlook.com>","Ezr <string121@live.com>"]` |
| `publisher`        | `string`                                   | The publisher name registered at [](alioth.org.cn)                                                                                                                                                                                                                        | `"cn.org.alioth"`                                                  |
| `license`          | string                                     | The license type name                                                                                                                                                                                                                                                     | `"MIT"`                                                            |
| `dependencies`     | `[string...]`                              | Dependency descriptions formated like this:<br/>`"package-id sections..."`<br/>This field will be applied to all packages, so the package variable are available:`$arch` and `$platform`, these two variables will be converted to certain exact string for each package. | `"cn.org.alioth.system-$platform-$arch:3.^ main doc dev"`          |
| `dev-dependencies` | `[string...]`                              | Development dependency descriptions. If the headers your development section provides needs headers in other package, specify them here.                                                                                                                                  |                                                                    |
| `packages`         | `{"package-full-name":package-descriptor}` | The field `packages` is an object, each key is a `package-full-name`, and the corresponding value is an object describes the package informations.                                                                                                                        | See `packages.package` for detail                                  |

Example:

~~~json
{
    "authors" : ["GodGnidoc <godgnidoc@outlook.com>"],
    "publisher" : "cn.org.alioth",
    "license" : "MIT",
    "dependencies" : [
        "cn.org.alioth.system-$platform-$arch:^ main"
    ],
    "packages" : {
        "stdlib-linux-x86_64" : {

        }, "stdlib-windows-x86_64" : {

        }, "stdlib-linux-arm7" : {

        }
    }
}
~~~

### Rules about package name

The package name with all informations included is called `package id`, example: `cn.org.alioth.stdlib-linux-x86_64:3.0.2`, it is consists of the following parts:
- publisher: `cn.org.alioth`
- package name: `stdlib`
- platform: `linux`
- architecture: `x86_64`  
  There's no rules about how to write an architecture name, but following the general style make your package easy to locate: `x86_64`,`x86`,`arm7`,`arm8`,`arm9`
- version: `3.0.2`

The `full package name` is used to locate a package in a namespace of one publisher, example: `stdlib-linux-x86_64`

### Packages.package

The configuration file `packages.json` is used to describe package informations built from this workspace. The section `packages` in that file contains multiple objects indexed by the package full name, each represent one package's configuration information.

Attributes in `package` object inherits from the `packages` object.

## Package

When installing a package from file system or from a remote repository, a config file named `package.json` will be generated, informations about this package will be written in it.

~~~json
{
  "name" : "<string>:package name",
  "version" : "<string>:version number, formatted as x.y.z",
  "repository" : "<uri>: the source where the package is from",
  "publisher" : "every publisher has its own namespace for packages",
  "authors" : [/*names of authors and their email addresses.*/],
  "license" : "license type name or path to the license file",
  "sections" : {
      "main" : {
        "resources" : [/** Paths to resources in this section */],
        "dependencies" : [/** If any dependency of this package is another alioth package, describe it here, alioth will help you to manage it. */]
      }
    }
}
~~~

If the name and the publisher of two packages are the same, these packages are two copy of one original package.

If this package is installed from remote repository, the field `repository` is given; When updating this package, the repository given in this config file will be searched first.

Example:

~~~json
{
  "name" : "http-linux-x86_64",
  "version" : "1.3.14",
  "repository" : "alioth://w3school.org/w3school",
  "author" : [
    "bob <bob@gmail.com>",
    "eric <eric@gmail.com>",
    "jack <jack@gmail.com>"
  ],
  "publisher" : "w3school@gmail.com",
  "license" : "doc/license.txt",
  "sections" : {
    "main" : {
      "resources" : [
        "arc/*.a",
        "inc/*.alioth"
      ],
      "dependencies" : [
        "alioth://alioth.org/alioth/stdlib-linux-x86_64:^"
      ]
    }, "doc" : {
      "resources" : [
        "doc/manual.md",
        "doc/api.md"
      ]
    }
  }
}
~~~

## Provide

The config file `provide.json` is automatically generated when compiling targets, and will be packed automatically.

The file `provide.json` describes which modules the targets provides.

Example:

~~~json
{
  "static" : {
    "arc/libstdlib.a" : [
      /** module names the target provides */
    ]
  }, "dynamic" : {
    "lib/libstdlib.so" : [
      "net",
      "memory",
      "string",
      "tuple",
      "map",
      "list"
    ]
  }
}
~~~

## Repository

> [TODO]

## Space

Generally, this file is automatically generated, the only field filled is the field `modules`; This field is used to speed up the dependencies closing.

You can manually insert the `mapping` section to map sub-spaces to other locations.

~~~json
{
  "mapping" : {
    "bin" : "ftps://company.org/target/bin/",
    "inc" : "http://company.org/libraries/inc/",
    "src" : "./main/src/"
  }, "modules" : {
    "iTalk" : {
      "deps": [
        {
          "alias" : "",
          "name" : "io",
          "from" : ""
        } , {
          "alias": "this",
          "name": "net",
          "from" : ""
        } , {
          "alias": "",
          "name": "memory"
        } , {
          "alias": "",
          "name": "string"
        }
      ],
      "code" : [
        {
          "mtim" : 1558544000,
          "name" : "talk.alioth",
          "size" : 1189,
          "space" : 64
        },{
          "mtim" : 1558541185,
          "name" : "talk.alioth",
          "size" : 470,
          "space" : 16
        }
      ]
    }
  }
}
~~~

If the `mapping` section is modified, the config file `space.json` will be packed when packing package.

The rule to map sub-spaces is simple, just write down uri which the compiler can handle.

## Targets

The config file `target.json` can be used to specify some attributes for targets.

You may change the target platform of this target, change the link rule of the target etc.

But this function is currently not supported.

> [TODO]