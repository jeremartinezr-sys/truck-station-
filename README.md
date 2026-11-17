# TRUCK STATION v1.0 — Plastic Sound Library

A first functional-in-design JUCE instrument plugin project (VST3 + Standalone), built as a hybrid synth/sample instrument.

## Current prototype features
- MIDI-triggered polyphonic oscillator synth (sine, triangle, saw, square).
- Load a WAV/AIFF sample as a playable sampler layer (root note C4 / MIDI 60).
- 30 preset names in the browser, with 10 distinct parameter recipes repeated across the list for this early build.
- Eight controls: Attack, Release, Color, Dust, Space, Wobble, Drive, Sample Mix.
- Clay-inspired retro interface and animated sleepy clay cat.
- Host-automatable parameters and saved parameter state.

## Important honest limitations
- This is source code, not a compiled/verified VST3 binary.
- The sample is currently layered with the synth voice; Sample Mix is a simple output gain/character control, not yet an independent sampler/synth crossfade.
- The 30 browser entries currently map to 10 core sound recipes. They are not yet 30 professionally sampled instruments.
- This is a basic oscillator engine; no wavetable, granular engine, sample slicing, or custom sample library yet.
- Sample loading is for the current plugin session; the sample file path is not yet persisted with the project.
- The current synthesis is a simple prototype and should be expanded/optimized before release.

## Build on Windows
1. Install Visual Studio 2022 with **Desktop development with C++**.
2. Install CMake 3.22 or newer.
3. Download JUCE 7 or JUCE 8 from https://juce.com/download/.
4. Put the JUCE source folder next to this CMakeLists file and name it `JUCE`.
5. Open PowerShell in this project folder and run:

   ```powershell
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

6. The plugin ends up in `build/TRUCK_STATION_artefacts/Release/VST3/TRUCK STATION.vst3` (it is a FOLDER, copy it whole).
7. Open PowerShell **as Administrator** in this folder and run `./install_vst3.ps1` (copies it to `C:\Program Files\Common Files\VST3`).
8. In FL Studio: Options > Manage plugins > Find installed plugins (enable "Verify plugins"). It appears under Installed > Instruments (or the VST3 section).

## Next development milestones
- Real 30–80 unique sound-design presets.
- Sample/synth blend and sample root/key range controls.
- User preset save/load browser.
- Built-in original one-shot and multisample library.
- Better filters, envelopes, modulation matrix, granular and sample slicing.
