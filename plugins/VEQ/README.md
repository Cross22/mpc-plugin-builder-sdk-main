# VEQ

An eight-band equalizer control prototype for Akai MPC standalone hardware, by BC.

The UI lays out eight color-coded bands with clickable enable indicators and
frequency, gain, and Q controls. Frequencies span 20 Hz–20 kHz, gain spans
-24 dB–+24 dB, and Q spans 0.1–10. The hardware Q-Links expose the three
continuous controls for four bands at a time. Use the `BAND1-4` and `BAND5-8`
tabs to switch which group of four is mapped to the Q-Links; all eight band
columns remain visible. A compact footer provides working master input gain
and placeholder Analyze / Debug buttons.

The graph is a static, flat 0 dB placeholder. The MPC scripted skin format
does not provide a way to render a parameter-driven combined response curve.
The band controls do not alter audio. The master control applies smoothed gain
from -24 dB to +24 dB to the stereo input. Analyze and Debug are UI placeholders
and do not initiate processing.

## Build and test

```
python tools/build.py plugins/VEQ
.sdk-tools/host_sim "plugins/VEQ/build/native/veq.dylib" test --seconds 3
```

## Controls

- Master: parameter zero (existing `gain` ID, retained for compatibility),
  applies smoothed input gain from -24 dB to +24 dB.
- Bands 1–8: enable/disable, frequency (20 Hz–20 kHz), gain (-24 dB–+24 dB),
  and Q (0.1–10).
- Analyze / Debug: placeholder toggles with no processing behavior.

## Project layout

- `plugin.json` - plugin identity, version, architecture-specific binaries.
- `src/VEQ.cpp` - parameter definitions and pass-through audio processing.
- `skins/` - hand-authored MPC touch UI, Q-Link mappings, and button images.
- `build/` - generated native/ARM builds and deployable plugin folder.
- `dist/` - zips produced by `tools/package.py`.
