# EVA Guardian — MVP Plan of Action

Chronological build-out plan for the EVA Guardian localized alert system MVP.
See [Moon2Mars_Individual.md](Moon2Mars_Individual.md) for the proposal this supports.

## Status

Scaffold complete and verified building/running via terminal (`cmake` + MSVC, no
Visual Studio IDE required):
- Async in-process pub/sub `MessageBus` (worker thread + queue)
- `TelemetryFrame` / `Alert` types
- `O2Monitor`, `PressureMonitor`, `ThermalMonitor` components (single threshold each)
- `main.cpp` wired up with a static vector of mock frames

## Steps

### 1. Live/scriptable simulator — DONE
`Simulator` publishes one combined `SuitTelemetryFrame` (o2 + pressure + thermal
together, matching how a real suit reports housekeeping telemetry per sample
interval) on the `"telemetry.suit"` topic every second on a background thread.
`main.cpp` reads stdin commands (`o2 <value>`, `pressure <value>`,
`thermal <value>`, `reset`, `quit`) and forwards them to `simulator.handleCommand()`
to override values live during a demo. All three monitors subscribe to the
same topic and each reads only the field it cares about.

### 2. Two-tier thresholds (Warning vs Critical)
Add a soft "Warning" band before the hard "Critical" band in each monitor so
alert severity escalates realistically instead of only ever firing Critical.

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
