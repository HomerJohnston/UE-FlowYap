# Yap for FlowGraph
Current status: alpha/early beta. **THIS README IS A WORK IN PROGRESS.**

## Introduction

Yap is a project-agnostic dialogue engine. It is being built by studying games like Monkey Island to try and recreate their capabilties. It is usable via any combination of blueprint or C++, although you will need a C++ project to build this plugin.

## Prerequisites:
- Get [FlowGraph](https://github.com/MothCocoon/FlowGraph)
- Clone Yap into your project
- Build and run.

## Quick-Start - Setup
- Yap requires a way to interface with your game. This is achieved by a "conversation broker". You must create a class (or blueprint) inheriting from UYapConversationBrokerBase and implementing its four interface functions. More info below.
- You must set your project to use your new conversation broker. Open Project Settings, go to Yap category, and assign the conversation broker class.

## Quick-Start - Usage
- The plugin adds the following primary Flow nodes:
  - Dialogue - The main character of the plugin. Dialogue nodes actually produce speech.
  - Start Convo - These are typically used to open conversation panels.
  - End Convo - These are typically used to shut down conversation panels.
- The plugin adds the following secondary Flow nodes:
  - Replace Fragment - this is used to swap out data from a line of a dialogue node.

## Conversation Broker Setup
The conversation broker class is used to help Yap communicate with your game. The broker inherits an interface which enforces five simple functions:
- OnConversationStart
- OnConversationEnd
- OnDialogueStart
- OnDialogueEnd
- AddPrompt

The way Yap works is simple: first, it creates an instance of the Conversation Broker on BeginPlay. Then when things happen, such as a "Start Convo" node runs, or a "Dialogue" node runs, it executes these functions on the conversation broker. Your task will be to make these functions do things to your game; the conversation start/end events should be used, for example, to open or close a conversation UI. The dialogue start/end events should be used, for example, to either populate text into the conversation panel (if a conversation is running) or to display speech above characters in your game (if no conversation is running). How you do this is completely up to you; Yap is little more than a simple engine which is running and emitting events.

