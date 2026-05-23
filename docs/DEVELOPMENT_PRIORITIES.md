# Development priorities by version

This project has repeatedly traded **progress** (how fast we can build), **functionality** (how much the app can do), and **performance** (how fast it runs on hardware). None of the three can be maximized at once; each prototype picked a different balance.

After **v0.3** — when the UI is in very good shape — the plan is to optimize for **performance** again.

---

## The triangle model

Every version can be drawn as a triangle with three corners:

| Corner | Meaning |
|--------|---------|
| **Progress** | Speed of development; shipping visible changes quickly; iteration over polish |
| **Functionality** | Features, settings, filters, metadata, screens, user-facing capability |
| **Performance** | Responsiveness on a real 3DS; I/O cost; rebuild time; redraw cost |

The **centre of gravity** (★) shows what that version optimized for. Closer to a corner or edge = stronger emphasis there.

---

## Version history (narrative)

### Initial prototype

Built for **speed** and assembled **very quickly**. That pass produced a lot of **progress** and the app felt **fast** on device. Scope was narrow by design.

### Next prototype

Gained a little **more functionality**, with **less focus on progress** — more time went into features than into raw shipping speed. **Performance was left untouched**; what worked stayed as-is.

### v0.2.0

Built with **functionality** and **rapid building** in mind. Filters, options, multiple screens, title database growth, and richer pick logic landed in a short window. **Performance took a hit** here: more work per reroll, full pool rebuilds, linear catalog scans, and heavier I/O were accepted as the cost of capability.

### v0.2.1 (current)

Focus is **UI**: layout, copy, inverted header/footer, paginator, filter section, fixed rows — built **rapidly** (**progress**) while preserving and surfacing **functionality**. **Performance has been hit hard** again: every interaction still triggers full `consoleClear()` redraws on top of the v0.2.0 backend costs, without backend optimization in this pass.

### v0.3 (planned)

UI should be in **very good shape**. Then **performance** becomes the primary optimization target again, rebalancing the triangle without dropping the features and UI already shipped.

---

## Priority balance diagrams

Each diagram uses the same three corners. **★** = centre of gravity for that version.

### Diagram 1 — Initial prototype

**Emphasis:** Progress + Performance. Minimal functionality.

```
                    PROGRESS
                   (rapid build)
                       ▲
                      / \
                     /   \
                    /     \
                   /   ★   \        ← high progress, high performance
                  /         \
                 /           \
                /_____________\
         FUNCTIONALITY     PERFORMANCE
            (minimal)          (strong)
```

| Priority | Level |
|----------|-------|
| Progress | ●●●●● High |
| Functionality | ● Low |
| Performance | ●●●●● High |

---

### Diagram 2 — Next prototype → v0.2.0

**Emphasis:** Functionality rises; progress stays strong through v0.2.0; performance starts to slip at v0.2.0.

**Next prototype** (lighter shift):

```
                    PROGRESS
                       ▲
                      / \
                     /   \
                    /  ★  \         ← functionality grows; perf unchanged
                   /       \
                  /_________\
         FUNCTIONALITY     PERFORMANCE
           (growing)         (held)
```

**v0.2.0** (same diagram family, centre moves):

```
                    PROGRESS
                       ▲
                      / \
                     / ★ \
                    /     \         ← functionality + rapid feature build
                   /       \
                  /_________\
         FUNCTIONALITY     PERFORMANCE
            (strong)           (weaker)
```

| Version | Progress | Functionality | Performance |
|---------|----------|---------------|-------------|
| Next prototype | ●●● Medium | ●●● Growing | ●●●● Unchanged |
| v0.2.0 | ●●●● High | ●●●● Strong | ●● Declining |

---

### Diagram 3 — v0.2.1 (current) → v0.3 (planned)

**v0.2.1 — today:** UI + progress + functionality; performance deprioritized.

```
                    PROGRESS
                       ▲
                      / \
                     /   \
                    /     \
                   /       \
                  /    ★    \       ← UI built fast; features exposed;
                 /             \      backend perf not addressed
                /_______________\
         FUNCTIONALITY     PERFORMANCE
            (strong)          (weak)
```

| Priority | Level |
|----------|-------|
| Progress | ●●●● High (rapid UI iteration) |
| Functionality | ●●●● Strong (filters, pages, settings) |
| Performance | ● Weak (full redraws + v0.2.0 backend cost) |

**v0.3 — planned:** UI complete; centre of gravity moves toward Performance.

```
                    PROGRESS
                       ▲
                      / \
                     /   \
                    /     \
                   /       \
                  /         \
                 /      ★    \      ← perf optimization pass
                /_______________\
         FUNCTIONALITY     PERFORMANCE
         (maintain)           (restore)
```

| Priority | Level |
|----------|-------|
| Progress | ●● Medium (polish + optimization) |
| Functionality | ●●●● Maintain |
| Performance | ●●●●● Primary focus |

---

## Evolution at a glance

```
Initial          Next           v0.2.0         v0.2.1         v0.3 (plan)
prototype        prototype
    │                │              │              │              │
    ▼                ▼              ▼              ▼              ▼
 progress        functionality  functionality   UI/progress    performance
     +                +              +              +              +
 performance      (perf flat)    (perf ↓)      (perf ↓↓)      (perf ↑)
 (function low)
```

---

## Performance notes (v0.2.1)

The UI pass did not fix underlying costs from v0.2.0. The main bottlenecks today are:

- **SMDH / icon file read** on every reroll (`title_smdh_load`)
- **Linear scan** of ~8,700-entry title database during pool rebuild
- **Full pool rebuild** on every filter toggle
- **`consoleClear()`** full-screen redraw on most interactions

These are documented in more detail in code review and [`MAIN_C_FUNCTIONALITY.md`](MAIN_C_FUNCTIONALITY.md). A v0.3 performance pass would likely target catalog lookup, pick caching, lazy metadata load, and partial UI updates first.

---

## Related

- [`VERSIONING.md`](VERSIONING.md) — semver rules and release mapping
- [`VERSION`](../VERSION) — current version file
