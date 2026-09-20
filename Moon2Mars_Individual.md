# Moon2Mars Proposal: EVA Guardian

## The Product
**The EVA Guardian Localized Alert System** is an Earth-independent telemetry processing engine designed to run directly on a lunar habitat's local server. 

Instead of waiting for high-latency, round-trip communications to Earth, this system ingests standardized data streams directly from astronauts' extravehicular activity (EVA) suits, monitoring critical metrics like oxygen flow, internal pressure, and thermal regulation. 

The architecture revolves around a streamlined, single-threaded backend that evaluates these incoming data packets in real-time. Upon detecting an anomaly, the engine immediately publishes alerts to a pub/sub message bus, instantly notifying the habitat crew and providing critical decision-support without delay. 

## What NASA Patent are we using?
**Space Link Extension Return Channel Frames (SLE-RCF) Software Library**  
*(NASA TOPS Identifier: GSC-TOPS-72)*

### What does it do?

At its core, the Space Link Extension Return Channel Frames (SLE-RCF) software library is an abstraction layer for space communications.

The SLE-RCF library acts like a universal API between the physical antennas and the mission control software. It handles the heavy lifting of taking that incoming raw radio data and packaging it into standardized, clean telemetry frames. It allows developers to simply request the data stream without worrying about the underlying radio hardware.

### Why should we use it?

By leveraging the [SLE-RCF library](https://technology.nasa.gov/patent/GSC-TOPS-72), the work of translating and standardizing raw space link signals is already handled. This allows the MVP development to focus entirely on the downstream logic—processing the telemetry and distributing the alerts—making it a highly achievable build for a tight three-week software sprint.

## What Shortfall does it target?
This product directly addresses **Need 1.6** from the 2026 Civil Space Shortfalls:

**"Provide in-situ decision support tools to address identified EVA hazards and risks on the lunar surface."**

By decoupling EVA monitoring from Earth-based mission control, the EVA Guardian ensures that lunar crews have autonomous, immediate insight into the safety and operational status of their team on the surface.

## How will it work?

Here is how the workflow would map out for an MVP:

**Data Ingestion:** We will simulate the SLE-RCF library's output. We feed the system a localized stream of mock, standardized telemetry frames representing the astronauts' EVA suits (e.g., oxygen levels, thermal readings, and internal pressure).

**The Core Engine:** A fast, single-threaded C++ backend will serve as the localized mission control processor. It will read these standardized frames in a continuous loop, evaluating the incoming metrics against safe operating thresholds.

**The Alert System:** If the single-threaded engine detects an anomaly (like a sudden pressure drop) it immediately generates a warning event.

**Data Distribution:** That warning event is pushed directly to a localized pub/sub message bus. Any interface subscribed to that bus, such as a visual dashboard inside the lunar habitat, receives the alert instantly without waiting for Earth to process it.