# TRUCK STATION V2 — SOUND SYSTEM 002

A JUCE-based VST3/Standalone synthesizer project. This version removes sample loading entirely and focuses on generated sound.

## Included in this source build

- Three-oscillator-style voice architecture (two main oscillators plus sub oscillator).
- Selectable oscillator character: sine, saw, square, triangle, FM-like and wavetable-like shapes.
- ADSR envelope controls.
- Resonant low-pass filter.
- Detune, sub level, FM amount, drive and LFO modulation.
- Chorus, reverb, output control and limiter.
- Mutate, Randomize and Chaos controls.
- Host-automatable parameters through AudioProcessorValueTreeState.
- Preset parameter state saved/restored by the host.
- Dark Y2K / tribal-inspired interface and an audio-reactive clay-cat visualization.
- No sampler and no sample-file loading.

## Important scope notes

This is a substantial **development prototype**, not a finished commercial instrument. The current build includes the core synthesis and modulation controls, but does not yet implement a true wavetable editor, a full modulation matrix, a working arpeggiator/step sequencer, preset browser/morph system, or per-effect routing. The `arp` parameters are reserved for the next engine pass. The cat currently reacts to output level (audio activity), not spectral pitch bands; its expression/shape reacts to the signal envelope.

The project has not been compiled or tested in this environment. Please build it locally and share the first compiler error if one appears. Do not assume the ZIP is already a loadable VST3.

## Build on Windows

1. Install Visual Studio 2022 (or just use the included GitHub Actions workflow, no install needed) and select **Desktop development with C++**.
2. Install CMake 3.22 or newer.
3. Install Git.
4. Open PowerShell in this folder.
5. Configure (CMake downloads JUCE 8.0.6 automatically):
   ```powershell
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   ```
6. Compile:
   ```powershell
   cmake --build build --config Release
   ```
7. The VST3 is created under the build tree, usually in:
   `build/TRUCK_STATION_artefacts/Release/VST3/`
8. Copy `TRUCK STATION.vst3` to:
   `%COMMONPROGRAMFILES%\VST3`
9. In FL Studio, open **Plugin Manager**, enable verification if needed, and rescan installed plugins.

## Notes

- The first CMake configure needs an internet connection because JUCE is fetched from GitHub.
- If CMake fails, copy the **first actual error** and the command output, not just the final summary line.
- This project intentionally uses `FetchContent_MakeAvailable(JUCE)` and a single `juce_add_plugin` target; do not add a second project() or duplicate JUCE target declarations.
