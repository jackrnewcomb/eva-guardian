# EVA Guardian — MVP Plan of Action

Chronological build-out plan for the EVA Guardian localized alert system MVP.
See [Moon2Mars_Individual.md](Moon2Mars_Individual.md) for the proposal this supports.

## Status

Scaffold complete and verified building/running via terminal (`cmake` + MSVC, no
Visual Studio IDE required):
- Async in-process pub/sub `MessageBus` (worker thread + queue)
- `SuitTelemetryFrame` (combined o2/pressure/thermal per suit per tick) / `Alert` types
- `O2Monitor`, `PressureMonitor`, `ThermalMonitor` components with Warning/Critical
  tiers and edge-triggered alerting (fire only on severity change, not every tick)
- `Simulator` + stdin command loop in `main.cpp` for live demo input

## Steps

### 1. Live/scriptable simulator — DONE
`Simulator` publishes one combined `SuitTelemetryFrame` (o2 + pressure + thermal
together, matching how a real suit reports housekeeping telemetry per sample
interval) on the `"telemetry.suit"` topic every second on a background thread.
`main.cpp` reads stdin commands (`o2 <value>`, `pressure <value>`,
`thermal <value>`, `reset`, `quit`) and forwards them to `simulator.handleCommand()`
to override values live during a demo. All three monitors subscribe to the
same topic and each reads only the field it cares about.

### 2. Two-tier thresholds (Warning vs Critical) — DONE
Each monitor now classifies each frame as Info (nominal) / Warning / Critical
and tracks its last published level, only publishing an alert when the level
changes. This also fixed a real bug: alerts previously fired on every tick
while a value stayed out of range, flooding the same console used for stdin
commands and making it impossible to type. Recovery back to nominal now emits
a one-time `[INFO]` alert instead of just going silent.

### 3. Live status dashboard subscriber
Add a second `"alerts"`/telemetry subscriber that tracks latest value per
suit/metric and re-renders a console table — stand-in for the habitat
dashboard described in the proposal.

### 4. Alert/telemetry logging polish
Human-readable timestamps, consistent log formatting, optional alert log file
output for demo artifacts.

### 5. Basic tests (CTest)
- `MessageBus` delivers published messages to subscribers
- Each monitor fires at the correct threshold boundary and stays silent when nominal

### 6. README + demo script
Terminal-based build/run quickstart, plus a scripted demo sequence
(nominal → warning → critical → recovery).

### 7. Final polish pass
Clean up TODOs, verify a clean build from scratch, rehearse the demo end-to-end.
