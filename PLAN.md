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

### 3. Multi-suit decision support + live dashboard — DONE
Threshold checking alone reads as "basic math" in a demo — this step is what
makes the system feel like an actual decision-support product:
- **Multi-suit**: run 3 independent simulated suits at once (`suit-1`,
  `suit-2`, `suit-3`), each with its own `Simulator` instance publishing on
  the shared `"telemetry.suit"` topic. Commands are now `<suit> <metric>
  <value>` / `<suit> reset` so you target a specific suit. Monitors track
  edge-triggered state **per suit ID** (`unordered_map<suitId, AlertSeverity>`)
  instead of a single `lastLevel_`, so suits don't clobber each other's state.
- **Recommended actions**: each `Alert` now carries a `metric` and
  `recommendedAction` string tied to its severity (e.g. critical O2 → "Abort
  EVA immediately: return to airlock and switch to backup O2 supply."),
  directly reflecting the proposal's "decision support" pitch rather than
  just flagging out-of-range values.
- **Live dashboard**: a `Dashboard` class subscribes to both `"telemetry.suit"`
  and `"alerts"`, tracks latest status per suit/metric, and renders an
  on-demand console table via the `dashboard` command — stand-in for the
  "visual dashboard inside the lunar habitat" from the proposal. In a real deployment a habitat likely
  already has its own dashboard software; the point is that the pub/sub bus
  is the integration surface — any subscriber (ours or an existing one) can
  plug into the same `"alerts"` topic without the engine owning the UI. Our
  console table is just a demo stand-in to prove that out.

### 4. Alert/telemetry logging polish
Human-readable timestamps, consistent log formatting, optional alert log file
output for demo artifacts.

### 5. Basic tests (CTest)
- `MessageBus` delivers published messages to subscribers
- Each monitor fires at the correct threshold boundary and stays silent when nominal
- Per-suit edge-triggered state doesn't cross-contaminate between suits

### 6. README + demo script
Terminal-based build/run quickstart, plus a scripted demo sequence
(nominal → warning → critical → recovery, across multiple suits).

### 7. Final polish pass
Clean up TODOs, verify a clean build from scratch, rehearse the demo end-to-end.
