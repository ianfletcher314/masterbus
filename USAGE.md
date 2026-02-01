# MasterBus - Usage Guide

**Mastering EQ and Compressor**

MasterBus is a professional mastering-grade channel strip featuring a comprehensive parametric EQ with high/low pass filters, shelving bands, and four parametric bands, plus a versatile mastering compressor with multiple modes. Complete with spectrum analyzer and loudness metering.

---

## Use Cases in Modern Rock Production

### Drum Bus Processing

MasterBus's precision EQ makes it excellent for surgical drum bus work.

**Drum Bus Shaping:**
- HPF: 30-40 Hz, 12 dB/oct (remove sub rumble)
- Low Shelf: 80 Hz, +2 dB (add weight)
- Band 1: 200-250 Hz, -2 dB, Q 1.0 (reduce mud)
- Band 2: 3-4 kHz, +1.5 dB, Q 1.0 (attack/snap)
- High Shelf: 10 kHz, +2 dB (air)
- Compressor: Threshold -18 dB, Ratio 2:1, Attack 20ms, Release Auto, Mode Glue

### Guitar Bus / Individual Tracks

Useful for precise EQ on guitar buses.

**Guitar Bus Polish:**
- HPF: 80-100 Hz, 12 dB/oct
- Band 1: 400 Hz, -1.5 dB, Q 1.5 (reduce muddiness)
- Band 2: 2.5 kHz, +1 dB, Q 1.0 (presence)
- High Shelf: 8 kHz, +1.5 dB (sparkle)
- Compressor: Bypass or very subtle (use Bus Glue instead)

### Bass Guitar

**Bass Tone Shaping:**
- HPF: 30-40 Hz, 12 dB/oct (remove sub rumble)
- Low Shelf: 60-80 Hz, +2-3 dB (fundamental weight)
- Band 1: 150-200 Hz, -2 dB, Q 1.0 (reduce boom)
- Band 2: 700-900 Hz, +1-2 dB, Q 1.5 (growl)
- Band 3: 2-3 kHz, +1 dB, Q 1.0 (articulation)
- LPF: 8-10 kHz (optional, reduce string noise)

### Vocals

MasterBus can serve as a more surgical alternative to VoxProc.

**Vocal Precision EQ:**
- HPF: 80-120 Hz, 24 dB/oct
- Band 1: 200-300 Hz, -2 dB, Q 1.5 (reduce proximity effect)
- Band 2: 800 Hz, -1 dB, Q 2.0 (reduce nasality if present)
- Band 3: 3 kHz, +2 dB, Q 1.0 (presence)
- Band 4: 5-6 kHz, -1 dB, Q 2.0 (tame harshness if needed)
- High Shelf: 12 kHz, +2 dB (air)

### Mix Bus / Mastering

This is MasterBus's primary purpose - final mix shaping and mastering.

**Mix Bus - Subtle Enhancement:**
- HPF: 25-30 Hz, 12 dB/oct (subsonic cleanup)
- LPF: 18-20 kHz, 6 dB/oct (gentle HF rolloff)
- Low Shelf: 60 Hz, +1-2 dB (add weight)
- Band 1: 200 Hz, -0.5-1 dB, Q 0.7 (reduce mud)
- Band 2: 3 kHz, +0.5-1 dB, Q 1.0 (presence)
- High Shelf: 10-12 kHz, +1-2 dB (air)
- Compressor: Mode Clean, Threshold -20 dB, Ratio 2:1, Attack 30ms, Release Auto, Mix 100%

**Mix Bus - More Character:**
- HPF: 30 Hz, 18 dB/oct
- Low Shelf: 80 Hz, +2 dB
- Band 1: 250 Hz, -1.5 dB
- Band 3: 4 kHz, +1.5 dB (more aggressive presence)
- High Shelf: 8 kHz, +2.5 dB
- Compressor: Mode Glue or Punch, Threshold -18 dB, Ratio 3:1, Attack 10ms, SC HPF 80 Hz

**Mastering - Rock/Alternative:**
- HPF: 25 Hz, 24 dB/oct
- Low Shelf: 50-60 Hz, +1-2 dB
- Band 1: 120 Hz, +0.5 dB, Q 0.8 (add punch)
- Band 2: 350 Hz, -0.5 dB, Q 0.7 (reduce boxiness)
- Band 3: 3 kHz, +0.5-1 dB, Q 1.0
- High Shelf: 12 kHz, +1.5 dB
- Compressor: Mode Glue, Threshold -15 dB, Ratio 2:1, Attack 20ms, Release Auto, SC HPF 60 Hz

**Mastering - Heavy Metal:**
- HPF: 30 Hz, 24 dB/oct
- Low Shelf: 60-80 Hz, +1.5 dB
- Band 1: 150 Hz, +1 dB, Q 1.0 (beef up kick)
- Band 2: 400 Hz, -1 dB, Q 1.0 (reduce mud)
- Band 3: 4-5 kHz, +1 dB (aggression)
- High Shelf: 10 kHz, +1 dB
- Compressor: Mode Punch, Threshold -16 dB, Ratio 3:1, Attack 10ms, SC HPF 100 Hz

---

## Recommended Settings

### Quick Reference - EQ Starting Points

| Frequency | Common Use | Cut/Boost |
|-----------|-----------|-----------|
| 25-40 Hz | Sub cleanup (HPF) | Cut |
| 50-80 Hz | Weight/power | Boost 1-3 dB |
| 100-200 Hz | Mud/boom | Cut 1-2 dB |
| 200-400 Hz | Boxiness/warmth | Cut for clarity, boost for warmth |
| 500-800 Hz | Body/honk | Cut if nasal |
| 1-2 kHz | Attack/presence | Boost for aggression |
| 2-4 kHz | Presence/clarity | Boost 0.5-2 dB |
| 4-6 kHz | Edge/harshness | Cut if harsh |
| 8-12 kHz | Air/brilliance | Boost 1-3 dB |
| 12-20 kHz | Ultra-highs | Subtle boost or cut |

### Compressor Mode Guide

- **Clean**: Transparent compression, minimal coloration - ideal for mastering acoustic/classical
- **Glue**: Adds cohesion and "togetherness" - the SSL-style mix bus sound
- **Punch**: Emphasizes transients, adds impact - great for rock/metal
- **Vintage**: Adds harmonic warmth and slower response - classic analog character

### Sidechain HPF Settings

- **60 Hz**: Subtle - prevents only the deepest bass from triggering compression
- **80-100 Hz**: Standard for rock/pop - lets kick punch through
- **120-150 Hz**: More aggressive - use when bass is triggering too much pumping

---

## Signal Flow Tips

### Where to Place MasterBus

1. **Mix Bus**: Insert on your main stereo bus as the primary mastering processor

2. **Before Limiting**: Place before any limiting (including Automaster)

3. **After Bus Glue**: If using both, Bus Glue first for glue compression, MasterBus for EQ and additional compression

### EQ Strategy: Subtractive First

1. Use HPF to remove unnecessary lows
2. Cut problem frequencies first (mud, harshness)
3. Then add subtle boosts where needed
4. Use narrow Q for cuts, wider Q for boosts

### Using Mid/Side Mode

- **EQ Mid/Side**: Process center (vocals, bass, kick) differently from sides (guitars, overheads)
  - Example: Boost high shelf on sides only for wider cymbals
  - Example: Cut low-mids on sides to clean up guitar mud

- **Comp Mid/Side**: Useful for controlling dynamics differently
  - Example: Less compression on sides preserves stereo width

---

## Combining with Other Plugins

### Standard Mix Bus Chain
1. **Bus Glue** - initial glue compression (2-3 dB GR)
2. **MasterBus** - EQ shaping and additional compression
3. **Automaster** - final limiting and loudness optimization

### Mastering Chain
1. **MasterBus** - main EQ and compression
2. **StereoImager** - stereo width adjustments
3. **Automaster** - limiting and loudness

### For Stem Mastering
- Use separate MasterBus instances on each stem (drums, bass, guitars, vocals)
- Light processing on each
- Final MasterBus on combined output

---

## Quick Start Guide

**Set up your mix bus in 60 seconds:**

1. Insert MasterBus on your stereo bus
2. Set **HPF**: 30 Hz, 18 dB/oct, Enabled
3. Set **Low Shelf**: 60 Hz, +1.5 dB, Enabled
4. Set **Band 1**: 200 Hz, -1 dB, Q 1.0, Enabled
5. Set **Band 3**: 3 kHz, +1 dB, Q 1.0, Enabled
6. Set **High Shelf**: 10 kHz, +1.5 dB, Enabled
7. Enable **Compressor**:
   - Mode: Glue
   - Threshold: -18 dB (adjust for 2-3 dB GR)
   - Ratio: 2:1
   - Attack: 20 ms
   - Release: Auto
   - SC HPF: 80 Hz
   - Mix: 100%
8. Set **Output Gain** to match bypass level
9. Watch the loudness meters and A/B with bypass

**Quick mastering setup in 60 seconds:**

1. Insert MasterBus on your master channel
2. Enable **HPF**: 25 Hz, 24 dB/oct
3. Set **Low Shelf**: 50 Hz, +1 dB
4. Set **Band 2**: 300 Hz, -0.5 dB, Q 0.7
5. Set **Band 3**: 3 kHz, +0.5 dB, Q 1.0
6. Set **High Shelf**: 12 kHz, +1.5 dB
7. **Compressor**:
   - Mode: Glue
   - Threshold: -15 dB (1-2 dB GR on peaks)
   - Ratio: 2:1
   - Attack: 30 ms
   - Release: Auto
   - SC HPF: 60 Hz
8. Match output to input
9. Follow with Automaster for limiting
