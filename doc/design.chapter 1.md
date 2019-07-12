---
title: Structure of Compiler
author: GodGnidoc
date: 2019/07/11
---

# 1. Chapter One: Structure of Compiler

- [1. Chapter One: Structure of Compiler](#1-Chapter-One-Structure-of-Compiler)
  - [1.1. Core workflow](#11-Core-workflow)
    - [1.1.1. Startup & initialize](#111-Startup--initialize)
    - [1.1.2. Determine work boundary](#112-Determine-work-boundary)
    - [1.1.3. Frontend work stream](#113-Frontend-work-stream)
    - [1.1.4. Backend work stream](#114-Backend-work-stream)
    - [1.1.5. Clean & exit](#115-Clean--exit)
  - [1.2. Engines](#12-Engines)
    - [1.2.1. SpaceEngine](#121-SpaceEngine)
    - [1.2.2. Compiler](#122-Compiler)
    - [1.2.3. DiagnosticEngine](#123-DiagnosticEngine)
    - [1.2.4. LexicalEngine](#124-LexicalEngine)
    - [1.2.5. SyntacticEngine](#125-SyntacticEngine)
    - [1.2.6. SemanticEngine](#126-SemanticEngine)

## 1.1. Core workflow

The core workflow is triggered when compiler is launched to compile some modules to be a target.

### 1.1.1. Startup & initialize

At the moment the command was executed, compiler should startup;

1. step 1:

    Collect inputs from command line arguments before nothing has been done.

1. step 2:

    Arguments from command line can be separated into two groups, a group of **pre-arguments** and a group of **post-arguments** for another.

1. step 3:
   
   Apply pre-arguments, then initialize compiler itself.

   Open resources or streams necessary according to pre-arguments.

   Read and apply options from configuration files.

1. step 4:
   
   Apply post-arguments to finish initialization of compiler.

### 1.1.2. Determine work boundary

Determine what modules are of to be compiled and their dependencies.

1. step 1:

    Update **workspace cache file** to detect distributing status of workspace.

1. step 2:

    Close dependencies of **target**, record all module placeholders which will be needed.

### 1.1.3. Frontend work stream

Read all source document of all modules to construct module fragments.

1. step 1:

    Go through all stages of parsing for each source documents to construct module fragments.

1. step 2:

    Collect fragments belong to same module to construct abstract module entity.

1. step 3:

    Validate semantics for definitions then for implementations.

    Generate low level intermediate representations.

### 1.1.4. Backend work stream

Translate low level intermediate representations into target machine code, then link all object files together into target.

1. step 1:
   
    Generate object files code using LLVM backend or native backend.

1. step 2:

    Locate all resources needed for linking then make up linking command.

1. step 3:

    Execute linking command.

### 1.1.5. Clean & exit

Clean environment and exit program.

## 1.2. Engines

According workflow described above, the following engines are designed.

### 1.2.1. SpaceEngine

SpaceEngine abstract the environment into space, handle data translations between compiler and any resource.

Provide accessing stream for any resource.

Can be configured by **pre-arguments** and configuration files.

### 1.2.2. Compiler

The `Compiler` is an engine named `Compiler` who controls all rules managing resources.

`Compiler` knows what to do and how; `Compiler` manages the strategy accessing resources according configuration files and **post-arguments**.

`Compiler` should manage all life cycle used resources of Target using an entity named `Target`.

### 1.2.3. DiagnosticEngine

The diagnostic engine provide a series of interfaces to generate and manage diagnostic messages.

Diagnostic engine can be configured by configuration files.

### 1.2.4. LexicalEngine

The lexical engine provides a series of interfaces to parse source code string into token sequences.

### 1.2.5. SyntacticEngine

The syntactic engine provides a series of interfaces to construct all kinds of nodes of syntax tree and finally the root node of syntax tree, aka module fragment.

### 1.2.6. SemanticEngine

The semantic engine can be used to get the following missions done.

- detect module entities from module fragment set for target.
- validate semantics of definitions in a target.
- validate semantics of implementations in a target.
- generate intermediate representations for LLVM backend or native backend.

semantic engine would have to modify content of syntax trees to fit language features.

> How to design native intermediate representation standard :  
>   
> semantic engine should assume that the backend can only process symbols of any entry but not names.  
> semantic engine should ensure that backend can translate any instructions gaven into machine code without semantic context.  
> the standard of the intermediate representation should support SSA.