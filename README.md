# EVA Guardian

**EVA Guardian** is an Earth-independent telemetry processing engine for a lunar habitat: it ingests
simulated EVA suit telemetry (O2, suit pressure, thermal), evaluates it locally against safety
thresholds, and publishes alerts with concrete recommended actions — without waiting on a
round-trip to Earth. It's an MVP built around NASA's [SLE-RCF library](https://technology.nasa.gov/patent/GSC-TOPS-72)
concept, targeting Need 1.6 from the 2026 Civil Space Shortfalls ("in-situ decision support tools
to address identified EVA hazards and risks on the lunar surface"). See
[Moon2Mars_Individual.md](Moon2Mars_Individual.md) for the full proposal and
[PLAN.md](PLAN.md) for the build-out history/design notes.

## What it does today

- **Simulates 3 suits** (`suit-1`, `suit-2`, `suit-3`) publishing combined O2/pressure/thermal
  telemetry once a second over an in-process async pub/sub message bus.
- **Reacts** to each suit's telemetry with Warning/Critical alerts (edge-triggered — only fires on
  a state change, not every tick), each carrying a concrete recommended action.
- **Predicts** trouble before it happens: projects each metric's rate of change forward and warns
  when a critical threshold is projected to be crossed soon, even before it's actually breached.
- **Correlates** signals across systems: if 2+ metrics on the same suit go abnormal at once, a
  composite alert escalates to "recommend full EVA abort," which no single monitor could raise
  alone.
- **Contrasts local vs. Earth-relayed response**: every alert is also echoed by a simulated
  "send it to Earth and wait" relay with an ~8s delay, directly demonstrating why local,
  Earth-independent processing matters.
- **Dashboards** live suit status on demand, and produces a **post-EVA debrief** report (per-suit
  min/max, time-in-severity, alert counts) when you quit.
- Logs every alert to `eva_guardian.log` in addition to the console.

## Build & run

Requires CMake and a C++17 compiler (tested with MSVC via Visual Studio's build tools). No IDE
needed — everything below runs from a terminal.

```powershell
cmake -S . -B build
cmake --build build --config Debug
.\build\Debug\eva_guardian.exe
```

Once running, interact via stdin:

| Command | Effect |
|---|---|
| `<suit> o2\|pressure\|thermal <value>` | Set a metric to an absolute value (e.g. `suit-1 o2 17.8`) |
| `<suit> o2\|pressure\|thermal drift <rate/s>` | Make a metric trend gradually (e.g. `suit-1 o2 drift -0.3`) |
| `<suit> reset` | Return a suit's metrics to nominal and clear any drift |
| `dashboard` | Print a live snapshot table of all suits |
| `quit` / `exit` | Stop and print the post-EVA debrief report |

## Running the tests

```powershell
ctest --test-dir build -C Debug --output-on-failure
```

## Running the automated demo

[demo.ps1](demo.ps1) drives `eva_guardian.exe` through a scripted, timed walkthrough that touches
every feature in one run, without you needing to type anything.

```powershell
powershell -ExecutionPolicy Bypass -File .\demo.ps1
```

(The `-ExecutionPolicy Bypass` flag only applies to this one invocation — it doesn't change your
system's script execution policy.)

### What to expect

The demo takes about 45-50 seconds and narrates each step in cyan before it happens:

1. **Baseline dashboard** — all three suits nominal (`[OK]` everywhere).
2. **Gradual O2 decline on suit-1** (`drift -0.3/s`) — watch a `TrendPredictor` `[WARNING]`
   ("Projected to reach critical O2 in ~Ns...") fire *before* the reactive `O2Monitor` warning,
   which itself later escalates to `[CRITICAL]`.
3. **Earth-independence contrast** — the same alerts you just saw reappear labeled
   `[EARTH RELAY, +8000ms delay]` about 8 seconds after their `[LOCAL]` counterparts, showing what
   would happen if this had to round-trip through Earth instead of resolving on the habitat server.
4. **Second simultaneous anomaly** (`suit-1 pressure 25`) — with two metrics abnormal on suit-1 at
   once, a `RiskAssessor` composite `[CRITICAL]` alert fires: "Multiple simultaneous anomalies
   detected... recommend full EVA abort."
5. **Dashboard** — suit-1 shows `[CRIT]` on multiple columns; `suit-2`/`suit-3` remain untouched.
6. **Recovery** (`suit-1 reset`) — `[INFO]` recovery alerts fire for O2, pressure, and the
   composite assessment. (You may also see a brief, spurious `TrendPredictor` warning here — a
   known cosmetic quirk of linear rate projection reacting to the instant reset jump.)
7. **Independent multi-suit anomalies** — `suit-2` thermal and `suit-3` pressure go critical at the
   same time, independently of each other and of suit-1's now-nominal state.
8. **Final dashboard** — reflects all three suits' current, independent status.
9. **Quit** — prints the **post-EVA debrief**: per-suit min/max values, time spent in each
   severity state, alert counts, predictive-warning counts, and composite-escalation counts.

After it finishes, check `eva_guardian.log` in the repo root for a plain-text record of every
alert raised during the run.
