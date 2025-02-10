**THIS README AND THE WIKI ARE UNDER CONSTRUCTION**

**YAP IS AN EARLY WORK IN PROGRESS. IT IS NOT PRODUCTION-READY, ALTHOUGH MOST COMMITS WILL BUILD AND RUN.**
# Yap for FlowGraph

&nbsp;

## Introduction

Yap is a project-agnostic dialogue engine running on FlowGraph. It is being built by studying games like Monkey Island to try and recreate their capabilties. It is usable via any combination of blueprint or C++, although you will need a C++ project to build this plugin.

Visit the wiki for documentation.

## Feature Overview

- Feature-rich dialogue nodes, containing:
  - Talk mode or Player Prompt mode
  - Activation limits
  - Configurable entrance conditions
  - Gameplay tag for lookup
  - Multiple dialogue fragments
  - Fragment run control (run all, select one, run til failure)
  - Normal flow output pin for normal talk mode
  - Individual output pins for player prompt mode
  - Bypass output pin (becomes available if there is a chance of the node or all fragments failing to activate)

Speech fragments each contain:
  - Selectable speaker
  - Selectable focus character (speech "directed at" character)
  - Mood tag
  - Written text dialogue
  - Title text (usually used for player prompts)
  - Activation limits
  - Configurable entrance conditions
  - Audio asset property
  - Automatic audio ID string
  - Configurable speaking time based on:
    - Audio asset length
    - Text length (word count ✕ project settings playback speed)
    - Manual time entry
  - Extra post-speech padding delay time
  - Optional child-safe data
  - Controllable progression settings
    - Automatic vs manual progression to next fragments
    - Skippable vs forced duration
  - Extra fields for dialogue text and title text to enter extra description for translators
  - Extra field for audio asset to enter stage directions for voice artists
  - Extra optional output pins to run more for when speaking begins and when speaking ends

Other general features:
  - Automatic hookup of audio assets to dialogue fragments (requires audio assets to include audio ID string in their name)
  - .PO Exporter for localization (exports a .PO file for all dialogue nodes in a single graph) - IN PROGRESS
  - Uses an interface you can apply to any object in your game to make it respond to dialogue events
  - Uses a "broker" class you can set up to interact with things from your game, such as:
    - Reading maturity settings
    - Reading dialogue playback settings
    - Interacting with 3rd party audio engines like WWise or FMod for editor functionality

&nbsp;


