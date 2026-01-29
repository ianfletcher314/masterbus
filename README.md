# MasterBus - Mastering EQ & Compressor

A professional mastering-grade EQ and compressor in a single plugin with comprehensive visual metering and analysis.

## Overview

MasterBus combines a transparent mastering EQ with a musical bus compressor, along with the visual feedback tools essential for mastering work. Designed for the final stage of production.

## Features

### Mastering EQ Section

#### Filter Types
- **High Pass Filter**: 10Hz-300Hz, 6/12/18/24dB slopes
- **Low Shelf**: 20Hz-500Hz, +/-12dB
- **Band 1 (Low)**: 30Hz-300Hz, parametric
- **Band 2 (Low-Mid)**: 100Hz-1kHz, parametric
- **Band 3 (Mid)**: 300Hz-5kHz, parametric
- **Band 4 (High-Mid)**: 1kHz-10kHz, parametric
- **High Shelf**: 2kHz-20kHz, +/-12dB
- **Low Pass Filter**: 5kHz-22kHz, 6/12/18/24dB slopes

#### EQ Features
- **Linear Phase Mode**: Zero phase distortion (adds latency)
- **Minimum Phase Mode**: Zero latency, natural phase
- **Mid/Side Processing**: EQ mid and side independently
- **Auto Gain**: Compensates for level changes
- **EQ Match**: Match to reference spectrum (stretch goal)

### Mastering Compressor Section

#### Compressor Controls
- **Threshold**: -40dB to 0dB
- **Ratio**: 1:1 to 10:1 (gentle mastering ratios)
- **Attack**: 0.1ms to 100ms
- **Release**: 50ms to 2000ms
- **Auto Release**: Program-dependent release
- **Knee**: Hard to soft (0-20dB)
- **Makeup Gain**: 0dB to 12dB

#### Compressor Modes
- **Clean/Transparent**: Minimal coloration
- **Glue**: Adds subtle harmonic warmth
- **Punch**: Enhanced transients
- **Vintage**: Modeled on classic hardware

#### Advanced Features
- **Mix (Parallel)**: 0-100% wet/dry
- **Sidechain HPF**: 20Hz-300Hz (avoid pumping from bass)
- **Sidechain Listen**: Hear what's triggering compression
- **Stereo Link**: 0-100% (independent to linked)
- **Mid/Side Compression**: Compress M/S independently

### Visual Analysis

#### Spectrum Analyzer
- **Pre/Post Toggle**: See EQ changes
- **FFT Size**: 1024 to 16384 bins
- **Averaging**: Adjustable smoothing
- **Peak Hold**: Shows maximum levels
- **Slope Options**: 0dB, 3dB, 4.5dB per octave

#### Level Metering
- **Input/Output Meters**: Peak + RMS
- **Loudness Meters**: LUFS (Integrated, Short-term, Momentary)
- **True Peak**: Intersample peak detection
- **Gain Reduction Meter**: Compressor activity
- **Stereo Correlation**: Phase relationship
- **Stereo Balance**: L/R level difference

#### Additional Displays
- **Dynamic Range Meter**: DR value
- **Waveform/History**: Scrolling waveform view
- **Loudness History**: LUFS over time graph

### Additional Features
- **A/B/C/D Comparison**: 4 setting slots
- **Reference Track**: Load reference for comparison
- **Mono Check**: Fold to mono button
- **Dim**: -20dB listening level
- **Bypass**: True bypass for A/B testing
- **Auto Gain Match**: Match loudness for fair comparison

## Signal Flow

```
Input -> Input Gain -> HPF/LPF -> EQ (Linear or Min Phase)
      -> Compressor -> Makeup -> Limiter (optional) -> Output

      [All stages have M/S options]
```

## UI Design

```
+------------------------------------------------------------------------+
|  MASTERBUS                                    [A][B][C][D]  [Preset v] |
+------------------------------------------------------------------------+
|                                                                         |
|  +----------------------- SPECTRUM ANALYZER --------------------------+ |
|  |                                                                     | |
|  |   ~~~~/\~~~~~/\_____/\~~~~~                                        | |
|  |                                                                     | |
|  |   [Pre/Post] [FFT Size v] [Slope v]                                | |
|  +---------------------------------------------------------------------+ |
|                                                                         |
|  +---------- EQ ----------+  +--------- COMPRESSOR ---------+  METERS  |
|  |                        |  |                              |          |
|  | [HPF]  [LS]  [1] [2]  |  |  [THRESH]  [RATIO]  [KNEE]  |  IN  OUT |
|  | [3]    [4]   [HS] [LPF]|  |  [ATTACK]  [RELEASE] [AUTO]  |  ||  || |
|  |                        |  |  [MAKEUP]  [MIX]    [MODE v] |  ||  || |
|  | [Linear Phase] [M/S]  |  |  [SC HPF]  [LINK]   [M/S]   |          |
|  | [BYPASS]              |  |  [BYPASS]                    |  LUFS    |
|  +------------------------+  +------------------------------+  [-14.2] |
|                                                                         |
|  LUFS: I:-14.2  S:-13.8  M:-12.4  |  TP:-0.3dB  |  DR:8  |  [MONO]   |
+------------------------------------------------------------------------+
```

## Implementation Plan

### Phase 1: Project Setup
- [ ] Create JUCE project
- [ ] Configure AU/VST3 targets
- [ ] Set up parameter system
- [ ] Basic shell with sections

### Phase 2: EQ DSP
- [ ] Implement biquad filters for each band
- [ ] Implement high/low pass filters
- [ ] Add shelf filters
- [ ] Linear phase mode (FFT-based)
- [ ] Mid/Side matrix encoding/decoding

### Phase 3: Compressor DSP
- [ ] Implement envelope follower
- [ ] Gain computer with variable knee
- [ ] Attack/release smoothing
- [ ] Auto-release algorithm
- [ ] Sidechain high-pass filter
- [ ] Stereo linking options
- [ ] M/S compression mode

### Phase 4: Metering & Analysis
- [ ] FFT spectrum analyzer
  - [ ] Windowing functions
  - [ ] Smoothing/averaging
  - [ ] Pre/post comparison
- [ ] Level meters
  - [ ] Peak metering
  - [ ] RMS metering
  - [ ] True peak detection
- [ ] Loudness metering (ITU-R BS.1770)
  - [ ] K-weighting filter
  - [ ] Gating
  - [ ] Integrated/Short-term/Momentary
- [ ] Correlation meter
- [ ] Gain reduction display

### Phase 5: UI Development
- [ ] Professional dark theme
- [ ] Spectrum analyzer display
  - [ ] OpenGL rendering for performance
  - [ ] Draggable EQ points
- [ ] Meter displays
- [ ] Knob components
- [ ] A/B/C/D system
- [ ] Preset management

### Phase 6: Advanced Features
- [ ] Linear phase EQ implementation
- [ ] Reference track loading
- [ ] Loudness matching
- [ ] Mono/dim monitoring
- [ ] Settings persistence

### Phase 7: Optimization
- [ ] SIMD optimization
- [ ] OpenGL for graphics
- [ ] Efficient FFT (use JUCE FFT or FFTW)
- [ ] Background thread for analysis

### Phase 8: Testing & Release
- [ ] Compare to reference plugins
- [ ] Test loudness meter accuracy
- [ ] Verify linear phase accuracy
- [ ] Create mastering presets
- [ ] Documentation

## Technical Specifications

- **Sample Rates**: Up to 192kHz
- **Bit Depth**: 64-bit internal processing
- **Latency**:
  - Minimum phase: Near-zero
  - Linear phase: ~20ms (configurable)
- **Loudness Standard**: ITU-R BS.1770-4, EBU R128
- **True Peak**: ITU-R BS.1770-4 compliant

## Loudness Standards Support

| Standard | Target | Range |
|----------|--------|-------|
| Streaming (Spotify, Apple) | -14 LUFS | -1dB TP |
| Broadcast (EBU R128) | -23 LUFS | -1dB TP |
| Film/TV | -24 LUFS | -2dB TP |
| Podcast | -16 LUFS | -1dB TP |

## Dependencies

- JUCE Framework 7.x
- C++17 or later
- OpenGL 3.2+ (for spectrum analyzer)

## Building

```bash
# macOS
cd Builds/MacOSX
xcodebuild -project MasterBus.xcodeproj -configuration Release
```

## License

MIT License
